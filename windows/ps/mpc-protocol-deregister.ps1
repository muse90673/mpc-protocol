# MPC Protocol Handler Deregistration Script
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

try {
    Write-Host "Removing mpc-hc:// protocol handler..."
    
    # Remove registry key
    $registryPath = "HKLM:\SOFTWARE\Classes\mpc-hc"
    if (Test-Path $registryPath) {
        Remove-Item -Path $registryPath -Recurse -Force
        Write-Host "Registry key removed."
    } else {
        Write-Host "Registry key does not exist, nothing to remove."
    }
    
    Write-Host "Deregistration complete!"
    
} catch {
    Write-Host "Error occurred: $_"
} finally {
    Read-Host "Press Enter to continue..."
}
