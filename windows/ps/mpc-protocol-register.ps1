# MPC Protocol Handler Registration Script
# This script requires administrator privileges

# 获取脚本所在目录
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
# 确保脚本路径不为空
if ([string]::IsNullOrEmpty($scriptPath)) {
    $scriptPath = Get-Location
    Write-Host "Using current directory: $scriptPath"
} else {
    # 自动切换到脚本所在目录
    Set-Location $scriptPath
    Write-Host "Changed to script directory: $scriptPath"
}

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "Error: This script requires administrator privileges."
    Write-Host "Please right-click the script and select 'Run as Administrator'."
    Read-Host "Press Enter to continue..."
    exit 1
}

# 查找MPC-HC可执行文件的函数
function Find-MPCExecutable {
    param(
        [string]$SearchPath
    )
    
    $possibleExes = @("mpc-hc64.exe", "mpc-hc32.exe", "mpc-hc.exe")
    
    # 首先在指定路径查找
    foreach ($exe in $possibleExes) {
        $exePath = Join-Path $SearchPath $exe
        if (Test-Path $exePath) {
            Write-Host "Found MPC-HC executable: $exePath"
            return $exePath
        }
    }
    
    # 如果在当前目录未找到，则尝试在父目录查找
    $parentPath = Split-Path -Parent $SearchPath
    if (-not [string]::IsNullOrEmpty($parentPath)) {
        foreach ($exe in $possibleExes) {
            $exePath = Join-Path $parentPath $exe
            if (Test-Path $exePath) {
                Write-Host "Found MPC-HC executable in parent directory: $exePath"
                return $exePath
            }
        }
    }
    
    # 如果仍未找到，尝试检查常见安装路径
    $commonPaths = @(
        "C:\Program Files\MPC-HC",
        "C:\Program Files (x86)\MPC-HC",
        "D:\apps\MPC-HC"
    )
    
    foreach ($path in $commonPaths) {
        foreach ($exe in $possibleExes) {
            $exePath = Join-Path $path $exe
            if (Test-Path $exePath) {
                Write-Host "Found MPC-HC executable in common path: $exePath"
                return $exePath
            }
        }
    }
    
    return $null
}

try {
    # 获取脚本目录
    $handlerPath = Join-Path $scriptPath "mpc-protocol.ps1"
    
    # 查找 MPC-HC可执行文件
    $mpcPath = Find-MPCExecutable -SearchPath $scriptPath
    
    # 检查所需文件是否存在
    if ($null -eq $mpcPath) {
        Write-Host "Error: MPC-HC executable not found"
        Write-Host "Current directory: $scriptPath"
        Write-Host "Please make sure this script is in the MPC installation directory."
        Write-Host "Or specify the path to MPC-HC manually by editing this script."
        Read-Host "Press Enter to continue..."
        exit 1
    }
    
    if (-not (Test-Path $handlerPath)) {
        Write-Host "Error: mpc-protocol.ps1 not found"
        Write-Host "Current directory: $scriptPath"
        Write-Host "Please make sure mpc-protocol.ps1 is in the same directory."
        Read-Host "Press Enter to continue..."
        exit 1
    }
    
    Write-Host "Registering mpc-hc:// protocol handler..."
    
    # 注册协议
    $registryPath = "HKLM:\SOFTWARE\Classes\mpc-hc"
    
    # 创建主键
    New-Item -Path $registryPath -Force | Out-Null
    Set-ItemProperty -Path $registryPath -Name "(Default)" -Value "URL:mpc-hc Protocol"
    Set-ItemProperty -Path $registryPath -Name "URL Protocol" -Value ""
    
    # 创建 DefaultIcon
    New-Item -Path "$registryPath\DefaultIcon" -Force | Out-Null
    Set-ItemProperty -Path "$registryPath\DefaultIcon" -Name "(Default)" -Value "`"$mpcPath`",0"
    
    # 创建 shell\open\command
    New-Item -Path "$registryPath\shell\open\command" -Force | Out-Null
    $commandValue = "powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -NonInteractive -NoProfile -File `"$handlerPath`" `"%1`""
    Set-ItemProperty -Path "$registryPath\shell\open\command" -Name "(Default)" -Value $commandValue
    
    Write-Host "Registration complete!"
    Write-Host "You can now use the following URL formats:"
    Write-Host "  mpc-hc://http://example.com/video.mp4"
    Write-Host "  mpc-hc://weblink?url=http://example.com/video.mp4"
    Write-Host "  mpc-hc://http//example.com/video.mp4 (Chrome 130+)"
    
} catch {
    Write-Host "Error occurred: $_"
} finally {
    Read-Host "Press Enter to continue..."
}
