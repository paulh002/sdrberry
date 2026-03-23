#!/bin/bash

# ==============================================================================
# SDRberry Audio Setup Script (Legacy/Pulse-Compatible Mode)
# ==============================================================================
# This script installs PipeWire, disables legacy PulseAudio daemon, 
# creates a virtual sink using 'pactl load-module', and configures the environment.
# ==============================================================================

set -e # Exit on error

USER_NAME=$(whoami)
HOME_DIR=$(eval echo ~$USER_NAME)

echo ">>> Starting SDRberry Audio Setup (Legacy Mode)..."
echo ">>> User: $USER_NAME"
echo ">>> Home: $HOME_DIR"

# ------------------------------------------------------------------------------
# 1. Install PipeWire and Dependencies
# ------------------------------------------------------------------------------
echo ""
echo "[1/6] Installing PipeWire, pavucontrol, and dependencies..."
sudo apt update
sudo apt install -y pipewire pipewire-audio-client-libraries pipewire-pulse \
    wireplumber libspa-0.2-bluetooth pulseaudio-utils alsa-utils pavucontrol

# ------------------------------------------------------------------------------
# 2. Disable Legacy PulseAudio Daemon
# ------------------------------------------------------------------------------
echo ""
echo "[2/6] Disabling legacy PulseAudio services to prevent conflicts..."
systemctl --user daemon-reload
systemctl --user disable --now pulseaudio pulseaudio.socket 2>/dev/null || true
sudo systemctl disable --now pulseaudio 2>/dev/null || true

# ------------------------------------------------------------------------------
# 3. Enable PipeWire Services
# ------------------------------------------------------------------------------
echo ""
echo "[3/6] Enabling PipeWire and WirePlumber..."
systemctl --user daemon-reload
systemctl --user enable --now pipewire pipewire-pulse pipewire.socket wireplumber

sleep 2

# ------------------------------------------------------------------------------
# 4. Configure Labwc Autostart to Create Virtual Sink
# ------------------------------------------------------------------------------
echo ""
echo "[4/6] Configuring labwc autostart to create virtual sink on login..."

AUTOSTART_FILE="$HOME_DIR/.config/labwc/autostart"
mkdir -p "$(dirname "$AUTOSTART_FILE")"

# Define the command to create the sink
# We add a small sleep to ensure PipeWire is fully ready
SINK_CMD1="sleep 3 && pactl load-module module-null-sink sink_name=sdrberry_monitor sink_properties=device.description='SDRberry_Monitor'"
SINK_CMD2="sleep 3 && pactl load-module module-null-sink sink_name=js8call_monitor sink_properties=device.description='JS8CALL_Monitor'"

# Check if the command already exists to avoid duplicates
if ! grep -q "sdrberry_monitor" "$AUTOSTART_FILE" 2>/dev/null; then
    echo "# Create SDRberry Virtual Sink" >> "$AUTOSTART_FILE"
    echo "$SINK_CMD1 &" >> "$AUTOSTART_FILE"
    echo "$SINK_CMD2 &" >> "$AUTOSTART_FILE"
    echo ">>> Added virtual sink creation to $AUTOSTART_FILE"
else
    echo ">>> Virtual sink command already present in $AUTOSTART_FILE. Skipping."
fi

# ------------------------------------------------------------------------------
# 5. Update .bashrc for Environment and Helper Tools
# ------------------------------------------------------------------------------
echo ""
echo "[5/6] Updating ~/.bashrc..."

if [ ! -f "$HOME_DIR/.bashrc.bak.sdrberry" ]; then
    cp "$HOME_DIR/.bashrc" "$HOME_DIR/.bashrc.bak.sdrberry"
    echo ">>> Backup created: ~/.bashrc.bak.sdrberry"
fi

BASHRC_BLOCK='
# --- SDRberry & PipeWire Legacy Setup Start ---
# Auto-detect WAYLAND_DISPLAY
if [ -z "$WAYLAND_DISPLAY" ] && [ -d "/run/user/$UID" ]; then
    export WAYLAND_DISPLAY=$(ls /run/user/$UID/wayland-* 2>/dev/null | head -n 1 | xargs basename 2>/dev/null)
fi

# Helper function to list and route SDRberry audio using legacy pactl
sdr-route() {
    echo "=== Available SDRberry Streams (Sink Inputs) ==="
    pactl list sink-inputs | grep -E "Index|application.name" | paste - - | sed "s/\t/ -> /g"
    echo ""
    echo "=== Available Sinks ==="
    pactl list sinks short
    echo ""
    if [ -n "$1" ] && [ -n "$2" ]; then
        echo "Moving stream #$1 to sink '$2'..."
        if pactl move-sink-input "$1" "$2"; then
            echo "Success!"
        else
            echo "Failed. Check indices above."
        fi
    else
        echo "Usage: sdr-route <stream_index> <sink_name_or_index>"
        echo "Example: sdr-route 78 sdrberry_monitor"
    fi
}

# Alias to quickly launch the volume control GUI
alias sdr-mixer="pavucontrol"
# --- SDRberry & PipeWire Legacy Setup End ---
'

if ! grep -q "SDRberry & PipeWire Legacy Setup Start" "$HOME_DIR/.bashrc"; then
    echo "$BASHRC_BLOCK" >> "$HOME_DIR/.bashrc"
    echo ">>> Environment variables and heSlper function added to ~/.bashrc"
else
    echo ">>> ~/.bashrc already contains SDRberry setup. Skipping append."
fi

# ------------------------------------------------------------------------------
# 6. Immediate Test (Optional)
# ------------------------------------------------------------------------------
echo ""
echo "[6/6] Attempting to create the virtual sink immediately for testing..."
if pactl load-module module-null-sink sink_name=sdrberry_monitor sink_properties=device.description="SDRberry_Monitor" 2>/dev/null; then
    echo ">>> Success! Virtual sink 'sdrberry_monitor' created for this session."
else
    echo ">>> Note: Could not create sink immediately (PipeWire might need a reboot first)."
    echo ">>> The sink will be created automatically after you reboot and log in."
fi
echo "[6/6] Attempting to create the virtual sink immediately for testing..."
if pactl load-module module-null-sink sink_name=js8call_monitor sink_properties=device.description="JS8CALL_Monitor" 2>/dev/null; then
    echo ">>> Success! Virtual sink 'js8call_monitor' created for this session."
else
    echo ">>> Note: Could not create sink immediately (PipeWire might need a reboot first)."
    echo ">>> The sink will be created automatically after you reboot and log in."
fi
# ------------------------------------------------------------------------------
# Completion Message
# ------------------------------------------------------------------------------
echo ""
echo "=============================================================================="
echo "Setup Complete!"
echo "=============================================================================="
echo ""
echo "IMPORTANT NEXT STEPS:"
echo "1. You MUST log out and log back in (or reboot) for the autostart changes"
echo "   to take effect and create the virtual sink permanently."
echo ""
echo "2. After logging back in, verify the sink exists:"
echo "   pactl list sinks short | grep sdrberry"
echo ""
echo "3. Usage:"
echo "   - Run 'sdr-route' to see streams and move them."
echo "   - Run 'sdr-mixer' (or pavucontrol) to manage volumes via GUI."
echo ""
echo "Enjoy SDRberry!"