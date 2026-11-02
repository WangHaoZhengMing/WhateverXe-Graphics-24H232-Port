WhateverXe 0.7.0 + Graphics 0.4.0 local port for macOS 15.8.2 (24H232).

Updated vendored WhateverXe to upstream d0b7fb6275ca51fde088fa7e91595462bd457ec9. Retains the early eDP boot-handoff fix, diagnostics and 2% PWM brightness floor. The previous fixed 24H232 call displacement patch is superseded by upstream instruction-boundary and target validation. The optional ControlCenter preference workaround remains available.

Graphics 0.4.0 payloads remain upstream originals, including the Metal/video compatibility fixes. Update both the EFI plugin and Graphics package when migrating from 0.3.2. Reboot is required; successful build/CPU tests do not establish hardware runtime compatibility or confirm that an application crash is fixed.

The pkg installs graphics components and stages the custom plugin under /Library/Application Support/WhateverXe-Port. It does not overwrite EFI automatically. Read README before installation. Other GPUs/panels are not hardware validated. Installer container is unsigned.
