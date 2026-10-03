#!/usr/bin/env python3
"""Fail-closed audit before publishing GoyaOS V2 artifacts publicly."""
from __future__ import annotations

import argparse
import gzip
import re
import shlex
import sys
import tarfile
from pathlib import Path, PurePosixPath

PRIVATE_KEY_RE = re.compile(rb"-----BEGIN (?:[A-Z0-9 ]+ )?PRIVATE KEY-----")
SSH_PUBLIC_KEY_RE = re.compile(
    rb"(?m)^(?:ssh-rsa|ssh-ed25519|ecdsa-sha2-[A-Za-z0-9-]+)\s+[A-Za-z0-9+/=]{20,}"
)
EMAIL_RE = re.compile(rb"(?i)\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}\b")
TOKEN_RES = (
    re.compile(rb"\bgh[pousr]_[A-Za-z0-9_]{30,}\b"),
    re.compile(rb"\bgithub_pat_[A-Za-z0-9_]{30,}\b"),
    re.compile(rb"\bAKIA[0-9A-Z]{16}\b"),
    re.compile(rb"(?i)\bBearer\s+[A-Za-z0-9._~+/=-]{24,}"),
)
BLOCKED_BASENAME_RE = re.compile(
    r"^(?:authorized_keys|id_(?:rsa|dsa|ecdsa|ed25519)(?:\.pub)?|"
    r"dropbear_(?:rsa|dss|ecdsa|ed25519)_host_key|ssh_host_.*_key)$",
    re.IGNORECASE,
)
LEGAL_NOTICE_BASENAME_RE = re.compile(
    r"^(?:licenses?|licences?|copying|notices?|copyrights?)(?:[._-].*)?$",
    re.IGNORECASE,
)
USER_DATA_TOP_LEVEL = {"etc", "root", "home", "var"}


def email_location_class(name: str) -> str:
    """Classify email matches without exposing member paths or their contents."""
    parts = PurePosixPath(name).parts
    if parts and parts[0].lower() in USER_DATA_TOP_LEVEL:
        return "user-config"
    # Do not rewrite or discard upstream notices; their public contact details are legal metadata.
    if parts and LEGAL_NOTICE_BASENAME_RE.fullmatch(parts[-1]):
        return "upstream-legal-notice"
    return "unclassified"


def fail(message: str) -> None:
    raise SystemExit(f"PUBLIC ARTIFACT AUDIT FAILED: {message}")


def normalized_name(name: str) -> str:
    while name.startswith("./"):
        name = name[2:]
    path = PurePosixPath(name)
    if path.is_absolute() or ".." in path.parts:
        fail("unsafe path in rootfs archive")
    return str(path)


def check_member_name(name: str) -> None:
    parts = PurePosixPath(name).parts
    if any(part == ".config" for part in parts):
        fail("Buildroot configuration file found in rootfs")
    if name.lower().endswith((".log", ".log.1", ".log.old")):
        fail("log file found in rootfs")
    if any(part == ".ssh" for part in parts):
        fail("SSH key directory found in rootfs")
    if BLOCKED_BASENAME_RE.match(PurePosixPath(name).name):
        fail("SSH key file found in rootfs")


def scan_archive(archive: tarfile.TarFile) -> tuple[int, int]:
    root_shadow: list[str] = []
    dropbear_args: list[str] = []
    files_scanned = 0

    for member in archive:
        name = normalized_name(member.name)
        if any(
            EMAIL_RE.search(value.encode("utf-8", "replace"))
            for value in (member.name, member.uname, member.gname)
            if value
        ):
            if email_location_class(name) == "user-config":
                fail("email pattern found in user/config member metadata (path withheld)")
            fail("email pattern found in rootfs member metadata (path withheld)")
        check_member_name(name)
        if not member.isfile():
            continue
        stream = archive.extractfile(member)
        if stream is None:
            fail("unreadable regular file in rootfs")
        files_scanned += 1
        carry = b""
        collected = bytearray() if name == "etc/shadow" else None
        while True:
            chunk = stream.read(1024 * 1024)
            if not chunk:
                break
            blob = carry + chunk
            if PRIVATE_KEY_RE.search(blob) or SSH_PUBLIC_KEY_RE.search(blob):
                fail("SSH/private-key material found in rootfs")
            if EMAIL_RE.search(blob):
                location = email_location_class(name)
                if location == "user-config":
                    fail("email pattern found in user/config data (member path withheld)")
                if location == "unclassified":
                    fail("email pattern found in unclassified rootfs member (member path withheld)")
            if any(pattern.search(blob) for pattern in TOKEN_RES):
                fail("credential/token pattern found in rootfs")
            if collected is not None:
                collected.extend(chunk)
                if len(collected) > 1024 * 1024:
                    fail("unexpectedly large shadow file")
            if name == "etc/default/dropbear":
                # This configuration is tiny; retain it to validate password auth is disabled.
                if len(carry) + len(chunk) > 65536:
                    fail("unexpectedly large Dropbear configuration")
                if collected is None:
                    collected = bytearray()
                collected.extend(chunk)
            carry = blob[-512:]

        if name == "etc/shadow":
            try:
                text = bytes(collected or b"").decode("utf-8")
            except UnicodeDecodeError:
                fail("shadow file is not valid UTF-8")
            rows = [line.split(":") for line in text.splitlines() if line]
            roots = [row for row in rows if len(row) >= 2 and row[0] == "root"]
            if len(roots) != 1 or roots[0][1] != "":
                fail("root password field is not empty")
            locked_fields = {"", "!", "!!", "*", "!*", "x"}
            if any(len(row) >= 2 and row[0] != "root" and row[1] not in locked_fields for row in rows):
                fail("non-root password hash/credential found in shadow")
            root_shadow.extend([row[1] for row in roots])

        if name == "etc/default/dropbear":
            try:
                text = bytes(collected or b"").decode("utf-8")
            except UnicodeDecodeError:
                fail("Dropbear configuration is not valid UTF-8")
            for line in text.splitlines():
                line = line.strip()
                if not line.startswith("DROPBEAR_ARGS="):
                    continue
                raw = line.split("=", 1)[1].strip()
                try:
                    args = shlex.split(raw)
                except ValueError:
                    fail("invalid Dropbear arguments")
                if len(args) == 1:
                    try:
                        args = shlex.split(args[0])
                    except ValueError:
                        fail("invalid Dropbear arguments")
                dropbear_args.extend(args)

    if len(root_shadow) != 1:
        fail("/etc/shadow missing or duplicate")
    if "-s" not in dropbear_args:
        fail("Dropbear password authentication is not disabled")
    return files_scanned, len(dropbear_args)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rootfs", type=Path, required=True)
    parser.add_argument("--zimage", type=Path, required=True)
    parser.add_argument("--boot", type=Path, required=True)
    parser.add_argument("--initramfs", type=Path, required=True)
    args = parser.parse_args()

    for path in (args.rootfs, args.zimage, args.boot, args.initramfs):
        if not path.is_file() or path.stat().st_size == 0:
            fail("required artifact missing or empty")

    try:
        with tarfile.open(args.rootfs, "r:") as archive:
            files_scanned, _ = scan_archive(archive)
    except (tarfile.TarError, OSError) as exc:
        fail(f"rootfs archive could not be read ({type(exc).__name__})")

    if args.boot.read_bytes()[:8] != b"ANDROID!":
        fail("boot image is not Android v0 format")
    try:
        with gzip.open(args.initramfs, "rb") as stream:
            if stream.read(6) != b"070701":
                fail("initramfs is not gzip-compressed newc cpio")
    except OSError:
        fail("initramfs gzip stream is invalid")

    print(
        "PASS: blank local root password; Dropbear password auth disabled; "
        "email policy passed (upstream legal notices preserved); no key/token/config/log indicators; "
        "Android boot and initramfs formats valid."
    )


if __name__ == "__main__":
    main()
