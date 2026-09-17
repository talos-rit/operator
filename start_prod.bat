@echo off
cd /d "%~dp0"

setlocal
rem Start the production Ichor deployment on Bluey.
rem WARNING: restarting the robot controller can physically move Bingo.

ssh pi@bluey.local "set -e; pkill -TERM -x erv || true; pkill -TERM -x ichor || true; for i in 1 2 3 4 5; do pgrep -x erv > /dev/null || pgrep -x ichor > /dev/null || break; sleep 1; done; if pgrep -x erv > /dev/null || pgrep -x ichor > /dev/null; then pkill -KILL -x erv || true; pkill -KILL -x ichor || true; sleep 1; fi; test -x '/home/pi/talos/operator/build/bin/ichor'; cd '/home/pi/talos/operator'; mkdir -p logs; nohup './build/bin/ichor' >> 'logs/ichor-launch.log' 2>&1 < /dev/null & sleep 2; pgrep -x ichor | wc -l | grep -qx 1; pgrep -a -x ichor"
set "exit_code=%ERRORLEVEL%"

if not "%exit_code%"=="0" (
  echo Failed to restart the production Ichor deployment.
)

exit /b %exit_code%
