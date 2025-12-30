$wjq = ".\wjq\build3\Release\wjq.exe"
$testJson = "wjq\tests\data\simple.json"
$testJsonl = "wjq\tests\data\mixed.jsonl"

function Show-Header {
    param($Text)
    Write-Host "`n=== $Text ===" -ForegroundColor Cyan
}

if (-not (Test-Path $wjq)) {
    Write-Host "Fout: wjq.exe niet gevonden op $wjq. Bouw het project eerst." -ForegroundColor Red
    exit 1
}

Show-Header "Demonstratie van Kleurenpaletten (Thema's)"
$themes = @("default", "dracula", "solarized", "monokai", "github", "minimal", "neon")

foreach ($theme in $themes) {
    Write-Host "Thema: $theme" -ForegroundColor Yellow
    & $wjq -t $theme $testJson
    Write-Host "-----------------------------------"
}

Show-Header "Compacte Output (-c)"
& $wjq -c $testJson

Show-Header "Aangepaste Indentatie (-i 8)"
& $wjq -i 8 $testJson

Show-Header "JSON Lines (JSONL) Ondersteuning"
Write-Host "Meerdere documenten in één bestand:" -ForegroundColor Yellow
& $wjq $testJsonl

Show-Header "Piped Input"
Write-Host "echo '{\`"piped\`": true}' | wjq" -ForegroundColor Yellow
"{`"piped`": true, `"status`": `"success`"}" | & $wjq

Show-Header "Kleurmodi (-m)"
Write-Host "Forceer 16 kleuren:" -ForegroundColor Yellow
& $wjq -m 16 $testJson
Write-Host "Forceer 256 kleuren:" -ForegroundColor Yellow
& $wjq -m 256 $testJson
Write-Host "Zet kleur uit (disabled):" -ForegroundColor Yellow
& $wjq -m disabled $testJson

Show-Header "Einde Demo"
