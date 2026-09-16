@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Deploy-ERV.ps1" -DeployDirectory "/home/pi/test"
set "exit_code=%ERRORLEVEL%"
if not "%exit_code%"=="0" (
  echo.
  echo ER-V deployment failed. Read the output above; the old binary was left running unless promotion completed.
  pause
)
exit /b %exit_code%
