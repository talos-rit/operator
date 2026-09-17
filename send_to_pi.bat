@echo off
cd /d "%~dp0"

setlocal
rem Deploy and start the ER-V development Operator from /home/pi/test.
rem WARNING: starting erv queues HOME and can physically move Bingo.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\Deploy-ERV.ps1"
set "exit_code=%ERRORLEVEL%"
if not "%exit_code%"=="0" (
  echo.
  echo ER-V development deployment failed. Read the output above.
  pause
)
exit /b %exit_code%
