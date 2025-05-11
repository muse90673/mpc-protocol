# MPC-HC Protocol Handler for Windows

这是一个 Windows 下的 MPC-HC 协议处理程序，支持通过点击 `mpc-hc://` 链接来用 MPC-HC 播放视频。

- 测试环境
  - 系统: Windows10
  - MPC-HC版本: [clsid2/mpc-hc v2.4.2](https://github.com/clsid2/mpc-hc/releases/tag/2.4.2)

## 功能特性

- 支持标准格式：`mpc-hc://http://example.com/video.mp4`
- 支持 Chrome 130+ 格式（自动修复）：`mpc-hc://http//example.com/video.mp4`
- 支持 weblink 格式：`mpc-hc://weblink?url=http%3A%2F%2Fexample.com%2Fvideo.mp4`

## 安装方式

### 方式一：PowerShell 脚本安装（推荐）

1. 从 [Releases](https://github.com/northsea4/mpc-protocol/releases) 页面下载最新的 [mpc-protocol-windows-powershell.zip](https://github.com/northsea4/mpc-protocol/releases/latest/download/mpc-protocol-windows-powershell.zip) 文件
2. 解压文件到 MPC-HC 安装目录（通常是 `C:\Program Files\MPC-HC`），确保3个ps1文件跟mpc-hc64.exe处于同一目录下
3. 以管理员身份运行 `mpc-protocol-register.ps1`（右键点击文件，选择"以管理员身份运行"）

**如果右键菜单没有"以管理员身份运行"选项**，可以尝试以下简单方法：

方法一：拖拽运行
   - 按 `Win + S` 搜索 "PowerShell"
   - 右键点击 "Windows PowerShell" 或 "PowerShell"，选择"以管理员身份运行"
   - 在PowerShell窗口中输入 `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass`，按回车
   - 将MPC-HC安装目录中的 `mpc-protocol-register.ps1` 文件直接拖拽到PowerShell窗口，按回车运行脚本
   - 在询问是否更改执行策略时选择"Y"
   - 💡 如果不能拖拽，请使用方法二 👇🏻

方法二：命令行运行
   - 按 `Win + X`，然后选择"Windows PowerShell (管理员)"或"终端(管理员)"
   - 在打开的管理员PowerShell窗口中，输入以下命令（请将路径替换为实际MPC-HC安装路径）：
   ```
   cd "C:\Program Files\MPC-HC"
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
   .\mpc-protocol-register.ps1
   ```
   - 在询问是否更改执行策略时选择"Y"

### 方式二：批处理脚本安装（暂不推荐）

1. 从 [Releases](https://github.com/northsea4/mpc-protocol/releases/latest) 页面下载最新的 [mpc-protocol-windows-bat.zip](https://github.com/northsea4/mpc-protocol/releases/latest/download/mpc-protocol-windows-bat.zip) 文件
2. 解压文件到 MPC-HC 安装目录（通常是 `C:\Program Files\MPC-HC`），确保3个bat文件跟mpc-hc64.exe处于同一目录下
3. 以管理员身份运行 `mpc-protocol-register.bat`（右键点击文件，选择"以管理员身份运行"）

### 方式三：可执行文件安装

#### 直接下载安装
1. 从 [Releases](https://github.com/northsea4/mpc-protocol/releases/latest) 页面下载最新的 [mpc-protocol-windows-exe.zip](https://github.com/northsea4/mpc-protocol/releases/latest/download/mpc-protocol-windows-exe.zip) 文件
2. 将下载的 zip 文件解压后把里面的文件复制到 MPC-HC 安装目录
3. 以管理员身份运行 `mpc-protocol-register.bat` （右键点击文件，选择"以管理员身份运行"）

#### 自行编译
1. 安装 MinGW-w64 工具链
   - Windows: 使用 [MSYS2](https://www.msys2.org/) 安装
   - macOS: 使用 Homebrew 安装 `brew install mingw-w64`
   - Linux: 使用包管理器安装，如 `sudo apt install gcc-mingw-w64`
2. 克隆仓库：`git clone https://github.com/northsea4/mpc-protocol.git`
3. 进入目录：`cd mpc-protocol/windows/exe/`
4. 编译：`./build.sh`

## 卸载

### PowerShell 脚本卸载
以管理员身份运行 `mpc-protocol-deregister.ps1`

### 批处理脚本卸载
以管理员身份运行 `mpc-protocol-deregister.bat`

### 可执行文件卸载
以管理员身份运行 `mpc-protocol-deregister.bat`

## 系统要求

- Windows 10 或更高版本
- MPC-HC media player 1.7.13 或更高版本，推荐使用 [clsid2/mpc-hc](https://github.com/clsid2/mpc-hc/releases)

## 故障排除

如果遇到问题：

1. 确保以管理员身份运行注册脚本
2. 检查 MPC-HC 是否已正确安装
3. PowerShell 版本的日志文件位置：
   - 正常日志：`%TEMP%\mpc-protocol.log`
   - 错误日志：`%TEMP%\mpc-protocol-error.log`
4. 如果链接点击无反应，请检查浏览器是否允许打开外部协议

## 安全说明

- 程序会验证 URL 格式，只允许 http:// 和 https:// 链接
- 不支持执行任意命令，仅支持打开视频链接

## 开发说明

提供了三种实现方式：
1. PowerShell 脚本（ps）：功能完整，易于维护
2. 批处理脚本（bat）：简单直接，暂不支持 weblink 格式
3. 可执行文件（exe）：性能最好，支持 x86_64 和 arm64 架构

选择建议：
- 一般用户：使用 PowerShell 版本
- 追求性能：使用可执行文件版本
- 系统兼容性要求高：使用批处理脚本版本（暂不推荐）