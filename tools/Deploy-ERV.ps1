[CmdletBinding()]
param(
    [string]$PiHost = "bluey.local",
    [string]$PiUser = "pi",
    [string]$DeployDirectory = "/home/pi/test"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$operatorRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$target = "$PiUser@$PiHost"

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$archive = Join-Path ([System.IO.Path]::GetTempPath()) "talos-operator-$stamp.tgz"
$remoteArchive = "/tmp/talos-operator-$stamp.tgz"
$stageDirectory = "/tmp/talos-operator-stage-$stamp"

function Invoke-Pi([string]$Command) {
    & ssh $target $Command

    if ($LASTEXITCODE -ne 0) {
        throw "Pi command failed with exit code $LASTEXITCODE."
    }
}

try {
    foreach ($command in @("ssh", "scp", "tar")) {
        if (-not (Get-Command $command -ErrorAction SilentlyContinue)) {
            throw "Required command '$command' is unavailable."
        }
    }

    Write-Host "[1/6] Packaging local Operator source..."

    Push-Location $operatorRoot
    try {
        & tar `
            --exclude=.git `
            --exclude=build `
            --exclude=build-offline `
            --exclude=htmlcov `
            -czf $archive .

        if ($LASTEXITCODE -ne 0) {
            throw "Could not create deployment archive."
        }
    }
    finally {
        Pop-Location
    }

    Write-Host "[2/6] Uploading source to Pi..."

    & scp $archive "${target}:$remoteArchive"

    if ($LASTEXITCODE -ne 0) {
        throw "Could not upload deployment archive."
    }

    Write-Host "[3/6] Building and testing candidate on Pi..."

    Invoke-Pi @"
set -e

rm -rf '$stageDirectory'
mkdir -p '$stageDirectory'

tar -xzf '$remoteArchive' -C '$stageDirectory'

python3 -m py_compile '$stageDirectory/systemd/erv-tty-setup'

cmake \
    -S '$stageDirectory' \
    -B '$stageDirectory/build' \
    -DBUILD_TESTING=ON

cmake --build '$stageDirectory/build' --parallel 2

ctest \
    --test-dir '$stageDirectory/build' \
    --output-on-failure

test -x '$stageDirectory/build/bin/erv'
"@

    Write-Host "[4/6] Replacing dev deployment..."

    Invoke-Pi @"
set -e

# Stop the existing dev ER-V.
pkill -TERM -x erv || true

for i in 1 2 3 4 5; do
    pgrep -x erv >/dev/null || break
    sleep 1
done

if pgrep -x erv >/dev/null; then
    pkill -KILL -x erv
    sleep 1
fi

# Preserve logs, but replace everything else in the dev deployment.
mkdir -p '$DeployDirectory'
mkdir -p '$DeployDirectory/logs'

find '$DeployDirectory' \
    -mindepth 1 \
    -maxdepth 1 \
    ! -name logs \
    -exec rm -rf {} +

# Copy the tested source tree into the deployment directory.
tar \
    -C '$stageDirectory' \
    --exclude=.git \
    --exclude=build \
    -cf - . \
    | tar -C '$DeployDirectory' -xf -

# Copy the already-tested binary.
mkdir -p '$DeployDirectory/build/bin'

install \
    -m 0755 \
    '$stageDirectory/build/bin/erv' \
    '$DeployDirectory/build/bin/erv'

# Install/update tty helper/service.
sudo -n install \
    -m 0755 \
    '$DeployDirectory/systemd/erv-tty-setup' \
    /usr/local/sbin/erv-tty-setup

sudo -n install \
    -m 0644 \
    '$DeployDirectory/systemd/erv-tty.service' \
    /etc/systemd/system/erv-tty.service

sudo -n systemctl daemon-reload
sudo -n systemctl enable erv-tty.service
sudo -n systemctl restart erv-tty.service
"@

    Write-Host "[5/6] Starting development ER-V..."

    Invoke-Pi @"
set -e

stty \
    -F /dev/ttyUSB0 \
    9600 cs8 -cstopb -parenb -crtscts -hupcl \
    raw -echo ixon ixoff

mkdir -p '$DeployDirectory/logs'

nohup '$DeployDirectory/build/bin/erv' \
    >> '$DeployDirectory/logs/erv-$stamp.log' \
    2>&1 \
    < /dev/null &

sleep 2
"@

    Write-Host "[6/6] Verifying ER-V..."

    Invoke-Pi @"
set -e

pgrep -a -x erv

test "`$(pgrep -x erv | wc -l)" -eq 1

echo
echo 'Serial ownership:'
fuser -v /dev/ttyUSB0 || true

echo
echo 'ActiveMQ listener:'
ss -ltn | grep -E '[:.]61616[[:space:]]'

echo
echo 'Recent ER-V log:'
tail -n 20 '$DeployDirectory/logs/erv-$stamp.log' || true
"@

    Write-Host ""
    Write-Host "ER-V dev deployment complete."
    Write-Host "Log: $DeployDirectory/logs/erv-$stamp.log"
}
finally {
    if (Test-Path $archive) {
        Remove-Item -LiteralPath $archive -Force
    }

    & ssh $target "rm -f '$remoteArchive'; rm -rf '$stageDirectory'" 2>$null
}