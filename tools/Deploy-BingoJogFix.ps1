param(
    [switch]$ConfirmHome,
    [string]$PiHost = 'bingo.local',
    [string]$KeyPath = "$env:USERPROFILE/.ssh/bluey_ed25519"
)
$ErrorActionPreference = 'Stop'
$sshOptions = @('-o', 'BatchMode=yes', '-o', 'ConnectTimeout=8', '-o', 'HostKeyAlias=bluey.local', '-o', 'IdentitiesOnly=yes', '-i', $KeyPath)
$remoteScript = '/home/pi/deploy-bingo-jog-fix.sh'
& scp @sshOptions (Join-Path $PSScriptRoot 'deploy-bingo-jog-fix.sh') "pi@${PiHost}:$remoteScript"
if ($LASTEXITCODE -ne 0) { throw 'Could not upload the deployment helper.' }
if (!$ConfirmHome) {
    & ssh @sshOptions "pi@$PiHost" "sh $remoteScript --check"
    if ($LASTEXITCODE -ne 0) { throw 'Bingo preflight failed.' }
    Write-Host 'To install after releasing controls and clearing the workspace, rerun with -ConfirmHome.'
    exit 0
}
Write-Host 'Native Operator startup will HOME Bingo. Release all held controls and keep the workspace clear.'
# Prevent desktop-held jog refreshes from surviving the Operator restart.
foreach ($robot in @('bluey', 'bingo')) {
    $body = @{ host = $robot; enabled = $false } | ConvertTo-Json -Compress
    Invoke-RestMethod -Method Post -Uri 'http://127.0.0.1:8000/api/control/auto-track' -ContentType 'application/json' -Body $body | Out-Null
}
Invoke-RestMethod -Method Post -Uri 'http://127.0.0.1:8000/api/cameras/select' -ContentType 'application/json' -Body '{"host":"bingo"}' | Out-Null
Invoke-RestMethod -Method Post -Uri 'http://127.0.0.1:8000/api/control/stop' -ContentType 'application/json' -Body '{}' | Out-Null
& ssh @sshOptions "pi@$PiHost" "sh $remoteScript --confirm-home"
if ($LASTEXITCODE -ne 0) { throw 'Deployment did not finish. Inspect the output before attempting any further restart.' }
$workspace = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$commanderDir = Join-Path $workspace 'commander'
$entrypoint = Join-Path $commanderDir '.venv/Scripts/commander-web.exe'
if (!(Test-Path -LiteralPath $entrypoint)) { throw "Commander entry point not found: $entrypoint" }
$listener = Get-NetTCPConnection -LocalPort 8000 -State Listen -ErrorAction SilentlyContinue | Select-Object -First 1
if ($listener) {
    $server = Get-CimInstance Win32_Process -Filter "ProcessId=$($listener.OwningProcess)"
    if ($server.CommandLine -notmatch 'commander-web') { throw 'Port 8000 belongs to another application; refusing to stop it.' }
    Stop-Process -Id $listener.OwningProcess
}
Start-Process -FilePath $entrypoint -WorkingDirectory $commanderDir -WindowStyle Hidden
Write-Host 'Commander restarted with browser-held jog refreshes. Refresh the page; tracking remains OFF.'
