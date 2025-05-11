# MPC Protocol Handler PowerShell Script

# 获取输入参数（必须放在脚本最开始处）
param(
    [Parameter(Mandatory=$true)]
    [string]$InputUrl
)

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

# Function: URL decode
function UrlDecode {
    param([string]$UrlEncodedString)
    
    Add-Type -AssemblyName System.Web
    return [System.Web.HttpUtility]::UrlDecode($UrlEncodedString)
}

# Function: Process URL
function ProcessUrl {
    param([string]$Url)
    
    Write-Host "Input URL: $Url"
    
    # Handle weblink format
    if ($Url.StartsWith("mpc-hc://weblink?url=") -or $Url.StartsWith("mpc-hc://weblink/?url=")) {
        Write-Host "Detected weblink format"
        $Url = $Url.Replace("mpc-hc://weblink?url=", "").Replace("mpc-hc://weblink/?url=", "")
        Write-Host "Extracted URL: $Url"
        $Url = UrlDecode $Url
        Write-Host "URL decoded: $Url"
        return $Url
    }
    
    # Remove mpc-hc:// prefix
    if ($Url -match "mpc-hc://(.+)") {
        $Url = $matches[1]
        Write-Host "Removed prefix: $Url"
    }
    
    # Fix Chrome 130+ format
    if ($Url.StartsWith("http//") -or $Url.StartsWith("https//")) {
        Write-Host "Fixing Chrome 130+ format"
        $Url = $Url.Replace("http//", "http://").Replace("https//", "https://")
        Write-Host "Fixed URL: $Url"
    }
    
    return $Url
}

# Function to find MPC-HC executable
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

# Main program
try {
    # 记录日志
    $logFile = "$env:TEMP\mpc-protocol.log"
    "$(Get-Date) - Script started, parameters: $InputUrl" | Out-File -Append $logFile
    "$(Get-Date) - Script path: $scriptPath" | Out-File -Append $logFile
    
    # Find MPC-HC executable
    $mpcPath = Find-MPCExecutable -SearchPath $scriptPath
    
    # Check if MPC exists
    if ($null -eq $mpcPath) {
        Write-Host "Error: MPC-HC executable not found"
        Write-Host "Current directory: $scriptPath"
        Write-Host "Please make sure this script is in the MPC installation directory."
        $errorMessage = "Error: MPC-HC executable not found in $scriptPath"
        $errorMessage | Out-File -Append "$env:TEMP\mpc-protocol-error.log"
        exit 1
    }
    
    "$(Get-Date) - MPC-HC found at: $mpcPath" | Out-File -Append $logFile
    
    # Process URL
    $processedUrl = ProcessUrl $InputUrl
    Write-Host "Final URL: $processedUrl"
    
    # Start MPC with no window
    Write-Host "Starting MPC..."
    Write-Host "Command line: '$mpcPath' '$processedUrl'"
    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = $mpcPath
    $startInfo.Arguments = "`"$processedUrl`""
    $startInfo.CreateNoWindow = $true
    $startInfo.UseShellExecute = $false
    $process = [System.Diagnostics.Process]::Start($startInfo)
    
    # Write logs to temp file for debugging
    "$(Get-Date) - Processed URL: $processedUrl" | Out-File -Append $logFile
    "$(Get-Date) - Process started with PID: $($process.Id)" | Out-File -Append $logFile
} catch {
    $errorMessage = "Error occurred: $_"
    $errorMessage | Out-File -Append "$env:TEMP\mpc-protocol-error.log"
    exit 1
}
