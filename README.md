# WhateverXe Graphics 0.3.2 — 本地移植包

本目录包含修改后的 WhateverXe 0.6.1 完整源码、MacKernelSDK 和 Lilu 编译依赖，以及官方 WhateverXe Graphics 0.3.2 原始 pkg。**实际修改的是 WhateverXe 插件；Graphics 中的 Apple 图形二进制保持原样，并非重新开源或重新编译 Apple 驱动。**

## 适用范围

仅针对 macOS 15.8.2 build 24H232。本机 i7-13700H / A7A0 / 1920×1200 eDP 已验证图形加速和 Metal 2。其他电脑的面板、电源、接口可能不同，不能保证兼容。pkg 会限制系统版本；硬件条件继续由驱动检查。

## 包含的修改

1. 24H232 的 IOProtectCacheMode 调用校验适配；保留其他内核校验。
2. 内屏固件启动画面专用端口检查，允许接管时模式暂为 0，保留运行期模式要求及电源、链路检查。
3. 诊断记录 WEXLocalPortDiagnostic / WEXLocalHandoffDiagnostic / WEXLocalBootPanelQualified。
4. 最低亮度设为 1311/65535（约 2% PWM），熄屏电源路径保持独立。此项已编译及测试，实际最低亮度尚待用户确认。
5. 可选控制中心绕行脚本 METAL_ERROR_MODE=5；这是用户偏好设置，不是 Metal 驱动修复，不自动全局应用。

## 在另一台机器安装

先备份该机器 EFI，准备可用的恢复启动方式。不要复制本机完整 config.plist。

1. 在本目录执行 `bash tools/install-efi.sh /Volumes/你的EFI分区/EFI/OC`，或手动复制 artifacts/WhateverXe.kext。脚本只备份和复制插件，不自动修改 config.plist。
2. 在该机 OpenCore 的 Kernel/Add 中启用 WhateverXe（在 Lilu 后），启用 AMFIPass 1.4.1 Mod，禁用 WhateverGreen。csr-active-config 使用 Data 03080000；boot-args 添加 `-allow3d -disablegfxfirmware`。
3. DVMT 建议 BIOS 预分配 96 MB；只有固定 60 MB 时才添加 `-igfxdvmt`，并在实际 iGPU PCI 路径加入 Data：framebuffer-patch-enable=01000000、framebuffer-stolenmem=0000C003。不要对其他显存布局照搬。
4. 从该 EFI 启动 macOS 后安装 target 中的 pkg，重启。pkg 保留官方图形安装流程，另把本地插件和说明放到 `/Library/Application Support/WhateverXe-Port`。**pkg 不自动挂载/改写 EFI，也不替你启用插件。**
5. 如控制中心出现已确认的 Target device architecture is nil 崩溃，以登录用户运行 `bash tools/controlcenter-workaround.sh`。撤销：`defaults delete com.apple.controlcenter METAL_ERROR_MODE`，然后重启控制中心。

已安装过 Graphics 0.3.2 的机器通常只需更新 EFI 插件。

## 构建

需要 Apple Command Line Tools、Python 3.9+。在本目录运行：

```
python3 tools/build.py --rebuild
```

重新编译插件、运行测试并打包到 target。仅重打包现有 artifacts：`python3 tools/build.py`。

安装包容器未签名/未公证；原始图形 payload 签名不变，本地 EFI 插件未签名。不要用自行编译的 AppleIntelTGLCompat 替换原始签名组件。

## 验证与回退

本次插件测试 158 项通过；不等于其他机器硬件验证。原始工程 LICENSE.txt 及依赖许可证保留在 source。图形 payload 的权利归原权利人；本目录没有 Apple 驱动源码。

EFI 回退：从 install-efi.sh 产生的桌面备份恢复 WhateverXe.kext 和需要恢复的配置。Graphics 卸载使用上游工具 https://github.com/b00t0x/WhateverXe-Graphics 。

上游：https://github.com/b00t0x/WhateverXe （0.6.1）、https://github.com/b00t0x/WhateverXe-Graphics （0.3.2）。

## GitHub 构建和发布

首次从 GitHub 克隆后，先运行 `python3 tools/bootstrap.py`，下载校验固定版本的依赖，再运行 `python3 tools/build.py --rebuild`。

`target`、`artifacts`、上游二进制 pkg 和编译依赖不提交 Git。Actions 使用 Intel macOS runner 构建及测试；推送 `v*` 标签会自动将 pkg 和 SHA256SUMS 上传到 GitHub Release。main 分支和手动触发只生成 Actions 构建产物。

这是独立的本地移植项目，不是上游官方版本。保留了 source/WhateverXe 中的上游版权和许可证。
