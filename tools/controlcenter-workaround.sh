#!/bin/bash
set -euo pipefail
# Run as the desktop user, not sudo. Optional, not a driver fix.
defaults write com.apple.controlcenter METAL_ERROR_MODE -int 5
killall ControlCenter || true
