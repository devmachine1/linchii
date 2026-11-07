# Backs up the Lichii user data (%APPDATA%\eden) before a rebuild or a new build's first boot.
# Usage: powershell -ExecutionPolicy Bypass -File tools\fork\backup-lichii.ps1 [-Root <dir>] [-Keep 5]
param([string]$Root = "$env:USERPROFILE\LichiiBackups", [int]$Keep = 5)

$src = Join-Path $env:APPDATA 'eden'

if (Get-Process lichii, eden-cli -ErrorAction SilentlyContinue) {
    Write-Error 'Feche o emulador antes do backup.'; exit 1
}

$dst = Join-Path $Root ('eden-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
robocopy $src $dst /E /COPY:DAT /R:1 /W:1 /NP /NFL /NDL /XD log crash_dumps game_list /XF *.tmp *.migrating | Out-Null
if ($LASTEXITCODE -ge 8) { Write-Error "robocopy falhou (codigo $LASTEXITCODE)"; exit 1 }

$saves = Get-ChildItem "$dst\nand\user\save" -Recurse -File -ErrorAction SilentlyContinue
$size  = (Get-ChildItem $dst -Recurse -File | Measure-Object Length -Sum).Sum / 1MB
"Backup: $dst  ({0:N1} MB, {1} arquivos de save)" -f $size, $saves.Count

# Only folders this script created (eden-<timestamp>) are rotated.
Get-ChildItem $Root -Directory -Filter 'eden-*' | Sort-Object Name -Descending |
    Select-Object -Skip $Keep | ForEach-Object { Remove-Item $_.FullName -Recurse -Force; "Removido backup antigo: $($_.Name)" }
