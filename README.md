# WhateverXe Graphics 0.4.1 — 本地移植包

本目录包含基于上游 WhateverXe 0.8.0（db438fdf4d5fe903f5dfbc65537ed3884e2ea980）的完整源码和本地修复。MacKernelSDK、Lilu 和官方 Graphics 0.4.1 pkg 由 bootstrap 下载并校验。**实际修改的是 WhateverXe 插件；Graphics 中的 Apple 图形二进制磁盘文件保持原样，并非重新开源或重新编译 Apple 驱动。**

## 适用范围

仅针对 macOS 15.8.2 build 24H232。旧版组合在 i7-13700H / A7A0 / 1920×1200 eDP 上已验证图形加速和 Metal 2；本次升级组合需重启后重新验证。其他电脑的面板、电源、接口可能不同，不能保证兼容。pkg 会限制系统版本；硬件条件继续由驱动检查。

## 包含的修改

1. 原先针对 24H232 的固定调用字节补丁由上游指令解码及调用目标检查接替，保留内核指令、参数与栈布局检查。
2. 内屏固件启动画面专用端口检查，允许接管时模式暂为 0，保留运行期模式要求及电源、链路检查。
3. 诊断记录 WEXLocalPortDiagnostic / WEXLocalHandoffDiagnostic / WEXLocalBootPanelQualified。
4. 最低亮度设为 1311/65535（约 2% PWM），熄屏电源路径保持独立。
5. 内核态运行时内存页补丁（挂钩 `_cs_validate_page`），在签名哈希校验通过后将 `AppleIntelTGLGraphicsMTLDriver` 偏移 `0x7ed87` 处的 `75 0d` 替换为 `90 90`，根治每个 Metal Command Buffer 泄漏约 72 KB POSH 缓冲区的缺陷（上游 #47，导致 `WindowServer` 等进程内存随运行时间无限增长），同时完整保留磁盘文件原始 Mod 签名链，无需 `amfi=0x80`（诊断属性：`WEXLocalMetalLeakFixReady` / `WEXLocalMetalLeakFixLive`）。
6. 可选控制中心绕行脚本 METAL_ERROR_MODE=5；这是用户偏好设置，不是 Metal 驱动修复，不自动全局应用。

## 在另一台机器安装

先备份该机器 EFI，准备可用的恢复启动方式。不要复制本机完整 config.plist。

1. 在本目录执行 `bash tools/install-efi.sh /Volumes/你的EFI分区/EFI/OC`，或手动复制 artifacts/WhateverXe.kext。脚本只备份和复制插件，不自动修改 config.plist。
2. 在该机 OpenCore 的 Kernel/Add 中启用 WhateverXe（在 Lilu 后），启用 AMFIPass 1.4.1 Mod，禁用 WhateverGreen。csr-active-config 使用 Data 03080000；boot-args 添加 `-allow3d -disablegfxfirmware`。
3. DVMT 建议 BIOS 预分配 96 MB；只有固定 60 MB 时才添加 `-igfxdvmt`，并在实际 iGPU PCI 路径加入 Data：framebuffer-patch-enable=01000000、framebuffer-stolenmem=0000C003。不要对其他显存布局照搬。
4. 从该 EFI 启动 macOS 后安装 target 中的 pkg，重启。pkg 保留官方图形安装流程，另把本地插件和说明放到 `/Library/Application Support/WhateverXe-Port`。**pkg 不自动挂载/改写 EFI，也不替你启用插件。**
5. 如控制中心出现已确认的 Target device architecture is nil 崩溃，以登录用户运行 `bash tools/controlcenter-workaround.sh`。撤销：`defaults delete com.apple.controlcenter METAL_ERROR_MODE`，然后重启控制中心。

从旧版升级时，应同时更新本项目 EFI 插件和 Graphics 0.4.1，避免新旧组件混用。完整保留上游签名 payload；新版包含 Metal/视频驱动及内核更新兼容性修复。

## 构建

需要 Apple Command Line Tools、Python 3.9+。在本目录运行：

```
python3 tools/build.py --rebuild
```

重新编译插件、运行测试并打包到 target。仅重打包现有 artifacts：`python3 tools/build.py`。

安装包容器未签名/未公证；原始图形 payload 签名不变，本地 EFI 插件未签名。不要用自行编译的 AppleIntelTGLCompat 替换原始签名组件。

## 验证与回退

本版本 209 项 CPU/mock 测试全部通过，包含保留的内屏、亮度与 Metal POSH 内存泄漏补丁回归；本机 24H232 真实内核的指令、调用目标及私有 pmap 访问器校验通过。测试不等于重启后或其他机器的硬件验证。原始工程 LICENSE.txt 及依赖许可证保留在 source。图形 payload 的权利归原权利人；本目录没有 Apple 驱动源码。

EFI 回退：从 install-efi.sh 产生的桌面备份恢复 WhateverXe.kext 和需要恢复的配置。Graphics 卸载使用上游工具 https://github.com/b00t0x/WhateverXe-Graphics 。

上游：https://github.com/b00t0x/WhateverXe （0.8.0）、https://github.com/b00t0x/WhateverXe-Graphics （0.4.1）。

## GitHub 构建和发布

首次从 GitHub 克隆后，先运行 `python3 tools/bootstrap.py`，下载校验固定版本的依赖，再运行 `python3 tools/build.py --rebuild`。

`target`、`artifacts`、上游二进制 pkg 和编译依赖不提交 Git。Actions 使用 Intel macOS runner 构建及测试；推送 `v*` 标签会自动将 pkg 和 SHA256SUMS 上传到 GitHub Release。main 分支和手动触发只生成 Actions 构建产物。

这是独立的本地移植项目，不是上游官方版本。保留了 source/WhateverXe 中的上游版权和许可证。
