#!/bin/bash
set -euo pipefail
[ "$(sw_vers -buildVersion)" = 24H232 ] || { echo 'Requires 24H232'; exit 1; }
[ "$#" = 1 ] || { echo 'Usage: bash install-efi.sh /Volumes/YOUR_EFI/EFI/OC'; exit 1; }
oc_dir="$1"
[ -f "$oc_dir/config.plist" ] && [ -d "$oc_dir/Kexts" ] || { echo 'Invalid OC directory'; exit 1; }
base_dir="$(cd "$(dirname "$0")" && pwd)"
plugin="$base_dir/WhateverXe.kext"
[ -d "$plugin" ] || plugin="$base_dir/../artifacts/WhateverXe.kext"
backup_dir="$(mktemp -d "$HOME/Desktop/WhateverXe-EFI-backup.XXXXXX")"
cp "$oc_dir/config.plist" "$backup_dir/config.plist"
if [ -d "$oc_dir/Kexts/WhateverXe.kext" ]; then cp -R "$oc_dir/Kexts/WhateverXe.kext" "$backup_dir/"; fi
# Copy this complete bundle; leave all other drivers and config settings intact.
mkdir -p "$oc_dir/Kexts/WhateverXe.kext"
cp -R "$plugin/Contents" "$oc_dir/Kexts/WhateverXe.kext/"
cmp "$plugin/Contents/MacOS/WhateverXe" "$oc_dir/Kexts/WhateverXe.kext/Contents/MacOS/WhateverXe"
sync
echo "Copied. Backup: $backup_dir"
echo 'Enable WhateverXe after Lilu in Kernel/Add; use AMFIPass Mod. Read README before reboot.'
