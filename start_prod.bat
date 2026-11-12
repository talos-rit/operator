@echo off
cd /d "%~dp0"

setlocal
rem Start the production serial ER-V deployment on Bluey.
rem WARNING: restarting the robot controller can physically move Bingo.

ssh pi@bluey.local "set -e; pkill -TERM -x erv || true; pkill -TERM -x ichor || true; for i in 1 2 3 4 5; do pgrep -x erv > /dev/null || pgrep -x ichor > /dev/null || break; sleep 1; done; if pgrep -x erv > /dev/null || pgrep -x ichor > /dev/null; then pkill -KILL -x erv || true; pkill -KILL -x ichor || true; sleep 1; fi; cd '/home/pi/talos/operator'; if [ ! -x './build/bin/erv' ]; then make erv; fi; test -x './build/bin/erv'; mkdir -p logs; stty -F /dev/ttyUSB0 9600 cs8 -cstopb -parenb -crtscts -hupcl raw -echo ixon ixoff; nohup './build/bin/erv' >> 'logs/erv-launch.log' 2>&1 < /dev/null & sleep 2; pgrep -x erv | wc -l | grep -qx 1; pgrep -a -x erv"
set "exit_code=%ERRORLEVEL%"

if not "%exit_code%"=="0" (
  echo Failed to restart the production serial ER-V deployment.
)

exit /b %exit_code%
