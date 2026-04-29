#!/bin/bash
ls /sys/class/drm/ | sed -n 's/^card[0-9]*-//p' | sort -u
for f in /sys/class/drm/card*-*/status; do
    name=$(basename "$(dirname "$f")" | sed 's/^card[0-9]*-//')
    echo "$name: $(cat "$f")"
done
