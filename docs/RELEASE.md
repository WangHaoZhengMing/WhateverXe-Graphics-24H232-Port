Local WhateverXe 0.6.1 + Graphics 0.3.2 port for macOS 15.8.2 (24H232).

Includes the qualified kernel-call adjustment, early eDP boot-handoff fix, diagnostics, and a 2% PWM brightness floor. Graphics payloads remain upstream originals. Optional ControlCenter workaround is documented.

The pkg installs graphics components and stages the custom plugin under /Library/Application Support/WhateverXe-Port; configure the target machine's OpenCore separately. It does not overwrite EFI automatically. Read README before installation. Other GPUs/panels are not hardware validated. Installer container is unsigned.
