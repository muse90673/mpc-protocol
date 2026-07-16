@echo off
@echo.

if not exist "%~dp0mpc-protocol.exe" (
  echo Warning: Can't find mpc-protocol.exe.
  echo Did you compile it successfully?
  @echo.
  pause
  exit /b
)

set "MPC_EXE="
if exist "%~dp0mpc-hc64.exe" (
  set "MPC_EXE=mpc-hc64.exe"
) else if exist "%~dp0mpc-hc.exe" (
  set "MPC_EXE=mpc-hc.exe"
)

if "%MPC_EXE%"=="" (
  echo Warning: Can't find mpc-hc64.exe or mpc-hc.exe in this directory.
  echo Please put these files in your MPC-HC directory and then run this file.
  @echo.
  pause
  exit /b
)

echo If you see "ERROR: Access is denied."
echo then you need to right click and use "Run as Administrator".
@echo.
echo Associating mpc-hc:// with mpc-protocol.exe...

reg add HKCR\mpc-hc /ve /t REG_SZ /d "URL:mpc-hc Protocol" /f
reg add HKCR\mpc-hc /v "URL Protocol" /t REG_SZ /d "" /f
reg add HKCR\mpc-hc\DefaultIcon /ve /t REG_SZ /d "%~dp0%MPC_EXE%,0" /f
reg add HKCR\mpc-hc\shell\open\command /ve /t REG_SZ /d "\"%~dp0mpc-protocol.exe\" \"%%1\"" /f

@echo.
echo MPC-HC protocol registration completed successfully!
pause
