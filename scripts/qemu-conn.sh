#!/usr/bin/env bash
set -euo pipefail

ISO="${ISO:-iso/ios.iso}"
SOCK="${SOCK:-/tmp/ioslink.sock}"
HUB="${HUB:-./scripts/ioslink-hub}"
HUB_LOG="${HUB_LOG:-/tmp/ioslink-hub.log}"
LOCK="${LOCK:-/tmp/ioslink-hub.lock}"

if [ ! -f "$ISO" ]; then
  echo "Error: ISO file '$ISO' not found!"
  exit 1
fi

start_hub() {
  echo "Starting routing hub at $SOCK (log: $HUB_LOG)"
  setsid "$HUB" --sock "$SOCK" -v >"$HUB_LOG" 2>&1 </dev/null &
  disown || true
}

# Auto-start hub if its socket isn't already there. A lockfile serializes
# concurrent `make conn` calls so only one ends up spawning the hub.
if [ ! -S "$SOCK" ]; then
  (
    flock 9
    if [ ! -S "$SOCK" ]; then
      start_hub
      for _ in $(seq 1 100); do
        [ -S "$SOCK" ] && break
        sleep 0.05
      done
    fi
  ) 9>"$LOCK"

  if [ ! -S "$SOCK" ]; then
    echo "Error: hub did not come up. See $HUB_LOG"
    exit 1
  fi
fi

echo "Connecting QEMU to hub at $SOCK"
exec qemu-system-x86_64 \
  -cdrom "$ISO" \
  -chardev socket,id=link,path="$SOCK" \
  -serial chardev:link \
  -vga virtio \
  "$@"
