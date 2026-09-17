@echo off
cd /d "%~dp0"
setlocal
rem Start the test ER-V deployment on Bluey.
rem WARNING: restarting erv queues HOME and can physically move Bingo.

ssh pi@bluey.local "set -e; pkill -TERM -x erv || true; for i in 1 2 3 4 5; do pgrep -x erv > /dev/null || break; sleep 1; done; if pgrep -x erv > /dev/null; then pkill -KILL -x erv; sleep 1; fi; test -x '/home/pi/test/build/bin/erv'; cd '/home/pi/test'; mkdir -p logs; nohup './build/bin/erv' >> 'logs/erv-launch.log' 2>&1 < /dev/null & sleep 2; pgrep -x erv | wc -l | grep -qx 1; pgrep -a -x erv"
set "exit_code=%ERRORLEVEL%"

if not "%exit_code%"=="0" (
  echo Failed to restart the test ER-V deployment.
)

exit /b %exit_code%
