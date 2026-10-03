#!/bin/sh
set -eu

TARGET_DIR=${1:?Buildroot did not pass TARGET_DIR}

# Clear any inherited root password hash so the local console login has the
# deliberately empty password field, even if stale state was copied from V1.
if [ -f "$TARGET_DIR/etc/shadow" ]; then
    sed -i 's/^root:[^:]*:/root::/' "$TARGET_DIR/etc/shadow"
    chmod 640 "$TARGET_DIR/etc/shadow"
fi

INITTAB="$TARGET_DIR/etc/inittab"
LAUNCHER=/usr/sbin/goya-shell-start
[ -x "$TARGET_DIR/usr/bin/goya-shell" ] || {
    echo "goya-shell binary missing from target rootfs" >&2
    exit 1
}
[ -x "$TARGET_DIR$LAUNCHER" ] || {
    echo "goya-shell launcher missing from target rootfs" >&2
    exit 1
}
if ! grep -Fqx "tty1::respawn:$LAUNCHER" "$INITTAB"; then
    printf '\n# GoyaOS V2 owns /dev/fb0 via virtual terminal tty1; serial getty runs on ttyS1.\ntty1::respawn:%s\n' "$LAUNCHER" >> "$INITTAB"
fi
