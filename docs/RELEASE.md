WhateverXe 0.8.0 + Graphics 0.4.1 local port for macOS 15.8.2 (24H232).

Updated vendored WhateverXe to upstream db438fdf4d5fe903f5dfbc65537ed3884e2ea980 (0.8.0) and WhateverXe-Graphics to v0.4.1. Retains the early eDP boot-handoff fix, diagnostics, 2% PWM brightness floor, and adds a runtime kernel `_cs_validate_page` memory patch fixing the 72 KB per-command-buffer POSH memory leak in `AppleIntelTGLGraphicsMTLDriver` (WindowServer / Metal clients, upstream #47) without altering on-disk Mod signatures. The optional ControlCenter preference workaround remains available.

Graphics 0.4.1 payloads remain upstream originals. Update both the EFI plugin and Graphics package together, then reboot.

The pkg installs graphics components and stages the custom plugin under /Library/Application Support/WhateverXe-Port. It does not overwrite EFI automatically. Read README before installation. Other GPUs/panels are not hardware validated. Installer container is unsigned.
