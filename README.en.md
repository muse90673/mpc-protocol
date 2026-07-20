English | [简体中文](README.md)

# MPC-HC Protocol Handler for Windows

A Windows protocol handler for MPC-HC that enables playing videos via `mpc-hc://` links.

- Test Environment
  - OS: Windows 10
  - MPC-HC Version: [clsid2/mpc-hc v2.4.2](https://github.com/clsid2/mpc-hc/releases/tag/2.4.2)

## Features

- Standard format: `mpc-hc://http://example.com/video.mp4`
- Chrome 130+ format (auto-fix): `mpc-hc://http//example.com/video.mp4`
- Weblink format: `mpc-hc://weblink?url=http%3A%2F%2Fexample.com%2Fvideo.mp4`

## Installation

### Method 1: PowerShell Script (Recommended)

1. Download the latest [mpc-protocol-windows-powershell.zip](https://github.com/northsea4/mpc-protocol/releases/latest/download/mpc-protocol-windows-powershell.zip) from the [Releases](https://github.com/northsea4/mpc-protocol/releases) page
2. Extract the files to the MPC-HC installation directory (usually `C:\Program Files\MPC-HC`), ensuring the 3 ps1 files are in the same directory as `mpc-hc64.exe`
3. Run `mpc-protocol-register.ps1` as Administrator (right-click the file and select "Run as Administrator")

**If the right-click menu does not have a "Run as Administrator" option**, try the following methods:

Method 1: Drag and Drop
   - Press `Win + S` and search for "PowerShell"
   - Right-click "Windows PowerShell" or "PowerShell" and select "Run as Administrator"
   - In the PowerShell window, type `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass` and press Enter
   - Drag the `mpc-protocol-register.ps1` file from the MPC-HC installation directory directly into the PowerShell window and press Enter to run the script
   - Choose "Y" when prompted to change the execution policy
   - 💡 If dragging is not supported, use Method 2 👇🏻

Method 2: Command Line
   - Press `Win + X`, then select "Windows PowerShell (Admin)" or "Terminal (Admin)"
   - In the Administrator PowerShell window, enter the following commands (replace the path with your actual MPC-HC installation path):
   ```
   cd "C:\Program Files\MPC-HC"
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
   .\mpc-protocol-register.ps1
   ```
   - Choose "Y" when prompted to change the execution policy

### Method 2: Batch Script (Not Recommended Currently)

1. Download the latest [mpc-protocol-windows-bat.zip](https://github.com/northsea4/mpc-protocol/releases/latest/download/mpc-protocol-windows-bat.zip) from the [Releases](https://github.com/northsea4/mpc-protocol/releases/latest) page
2. Extract the files to the MPC-HC installation directory (usually `C:\Program Files\MPC-HC`), ensuring the 3 bat files are in the same directory as `mpc-hc64.exe`
3. Run `mpc-protocol-register.bat` as Administrator (right-click the file and select "Run as Administrator")

### Method 3: Executable File

#### Direct Download
1. Download the latest [mpc-protocol-windows-exe.zip](https://github.com/northsea4/mpc-protocol/releases/latest/download/mpc-protocol-windows-exe.zip) from the [Releases](https://github.com/northsea4/mpc-protocol/releases/latest) page
2. Extract the downloaded zip file and copy the contents to the MPC-HC installation directory
3. Run `mpc-protocol-register.bat` as Administrator (right-click the file and select "Run as Administrator")

#### Build from Source
1. Install the MinGW-w64 toolchain
   - Windows: Use [MSYS2](https://www.msys2.org/)
   - macOS: Use Homebrew `brew install mingw-w64`
   - Linux: Use your package manager, e.g., `sudo apt install gcc-mingw-w64`
2. Clone the repository: `git clone https://github.com/northsea4/mpc-protocol.git`
3. Navigate to the directory: `cd mpc-protocol/windows/exe/`
4. Build: `./build.sh`

## Uninstallation

### PowerShell Script
Run `mpc-protocol-deregister.ps1` as Administrator

### Batch Script
Run `mpc-protocol-deregister.bat` as Administrator

### Executable File
Run `mpc-protocol-deregister.bat` as Administrator

## System Requirements

- Windows 10 or later
- MPC-HC media player 1.7.13 or later; [clsid2/mpc-hc](https://github.com/clsid2/mpc-hc/releases) is recommended

## Troubleshooting

If you encounter issues:

1. Ensure you run the registration script as Administrator
2. Verify that MPC-HC is properly installed
3. Log file locations for the PowerShell version:
   - Normal log: `%TEMP%\mpc-protocol.log`
   - Error log: `%TEMP%\mpc-protocol-error.log`
4. If clicking links has no effect, check whether your browser allows opening external protocols

## Security Notes

- The program validates URL formats and only allows `http://` and `https://` links
- Arbitrary command execution is not supported; only video links can be opened

## Development Notes

Three implementations are provided:
1. PowerShell script (ps): Full-featured and easy to maintain
2. Batch script (bat): Simple and straightforward, does not currently support weblink format
3. Executable file (exe): Best performance, supports x86_64 and arm64 architectures

Recommendations:
- General users: Use the PowerShell version
- Performance-focused: Use the executable version
- High system compatibility requirements: Use the batch script version (not recommended currently)
