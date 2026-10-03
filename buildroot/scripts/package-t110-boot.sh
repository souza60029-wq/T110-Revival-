#!/bin/sh
set -eu

if [ "$#" -ne 5 ]; then
    echo "Usage: $0 <zImage> <rootfs.tar> <bootimg.cfg> <boot.img> <initramfs.cpio.gz>" >&2
    exit 2
fi

ZIMAGE=$1
ROOTFS_TAR=$2
BOOTCFG=$3
BOOTIMG=$4
RAMDISK=$5

for file in "$ZIMAGE" "$ROOTFS_TAR" "$BOOTCFG"; do
    [ -s "$file" ] || { echo "Missing input: $file" >&2; exit 1; }
done

mkdir -p "$(dirname "$BOOTIMG")" "$(dirname "$RAMDISK")"
WORK=$(mktemp -d)
cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT HUP INT TERM
ROOT="$WORK/rootfs"
mkdir -p "$ROOT"

# Reject absolute and parent-traversal members before extraction.
python3 - "$ROOTFS_TAR" <<'PY'
import sys, tarfile
from pathlib import PurePosixPath
with tarfile.open(sys.argv[1], "r:*") as archive:
    for member in archive.getmembers():
        name = member.name
        while name.startswith("./"):
            name = name[2:]
        path = PurePosixPath(name)
        if path.is_absolute() or ".." in path.parts:
            raise SystemExit("unsafe path in rootfs archive")
PY

tar -xpf "$ROOTFS_TAR" --no-same-owner -C "$ROOT"
[ -x "$ROOT/sbin/init" ] || { echo "rootfs has no executable /sbin/init" >&2; exit 1; }
[ -x "$ROOT/usr/bin/goya-shell" ] || { echo "rootfs has no goya-shell" >&2; exit 1; }

# The build uses an intentionally empty local root password, never a known hash.
python3 - "$ROOT/etc/shadow" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1])
if p.exists():
    roots=[line for line in p.read_text().splitlines() if line.startswith("root:")]
    if len(roots)!=1 or roots[0].split(":")[1] != "":
        raise SystemExit("root shadow field is not empty; refusing to package")
PY

# Preserve the Buildroot root tree; cpio assigns numeric ownership 0:0.
(
    cd "$ROOT"
    find . -print0 | LC_ALL=C sort -z | cpio --null --create --format=newc --owner=0:0 --quiet
) | gzip -n -9 > "$RAMDISK"

abootimg --create "$BOOTIMG" -f "$BOOTCFG" -k "$ZIMAGE" -r "$RAMDISK"

# Android v0 uses SHA-1 over kernel/ramdisk/second-stage data and their sizes.
# Patch the digest field, then verify every payload and the exact V1-compatible header.
python3 - "$BOOTIMG" "$ZIMAGE" "$RAMDISK" "$BOOTCFG" <<'PY'
from pathlib import Path
import gzip, hashlib, struct, sys
boot_path, kernel_path, ramdisk_path, cfg_path = map(Path, sys.argv[1:])
boot=bytearray(boot_path.read_bytes())
kernel=kernel_path.read_bytes()
ramdisk=ramdisk_path.read_bytes()
if boot[:8] != b"ANDROID!": raise SystemExit("not an Android boot image")
kernel_size, kernel_addr, ramdisk_size, ramdisk_addr, second_size, second_addr, tags_addr, page_size = struct.unpack_from("<8I", boot, 8)
if page_size != 2048: raise SystemExit(f"unexpected page size {page_size}")
if kernel_size != len(kernel) or ramdisk_size != len(ramdisk): raise SystemExit("header payload size mismatch")
kernel_off=page_size
ramdisk_off=((kernel_off+kernel_size+page_size-1)//page_size)*page_size
second_off=((ramdisk_off+ramdisk_size+page_size-1)//page_size)*page_size
if bytes(boot[kernel_off:kernel_off+kernel_size]) != kernel: raise SystemExit("embedded zImage mismatch")
if bytes(boot[ramdisk_off:ramdisk_off+ramdisk_size]) != ramdisk: raise SystemExit("embedded initramfs mismatch")
if gzip.decompress(ramdisk)[:6] != b"070701": raise SystemExit("initramfs is not gzip-compressed newc cpio")
second=bytes(boot[second_off:second_off+second_size])
digest=hashlib.sha1(kernel+struct.pack("<I",len(kernel))+ramdisk+struct.pack("<I",len(ramdisk))+second+struct.pack("<I",second_size)).digest()
boot[576:608]=digest+bytes(12)

expected={"kerneladdr":0x10008000,"ramdiskaddr":0x11000000,"secondaddr":0x10f00000,"tagsaddr":0x10000100,"pagesize":0x800}
config={}
for line in cfg_path.read_text().splitlines():
    if "=" in line:
        key,value=line.split("=",1)
        config[key.strip()]=value.strip()
for key,value in expected.items():
    if key not in config or int(config[key],0)!=value: raise SystemExit(f"unexpected {key} in boot config")
cmdline=config.get("cmdline","")
header_cmd=(bytes(boot[64:576])+bytes(boot[608:1632])).split(b"\0",1)[0].decode("ascii")
if header_cmd != cmdline: raise SystemExit("boot header cmdline mismatch")
if "console=ttyS1,115200" not in header_cmd or "console=tty0" not in header_cmd or "init=/sbin/init" not in header_cmd:
    raise SystemExit("required console/init args missing")
if second_size != 0: raise SystemExit("unexpected second-stage payload")
if boot[576:596] != digest: raise SystemExit("Android boot ID mismatch")
if len(boot) != int(config["bootsize"],0): raise SystemExit("boot image size differs from configured partition size")
boot_path.write_bytes(boot)
print("Android boot v0 verified: header, addresses, exact cmdline, zImage, initramfs and SHA-1 ID")
PY

chmod 644 "$BOOTIMG" "$RAMDISK"
printf 'boot.img: %s bytes\n' "$(wc -c < "$BOOTIMG")"
printf 'initramfs: %s bytes\n' "$(wc -c < "$RAMDISK")"
