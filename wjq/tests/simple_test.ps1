$wjq = "..\build\wjq\Release\wjq.exe"
Write-Host "Testing wjq executable..."
Write-Host "Path: $wjq"
Write-Host "Exists: $(Test-Path $wjq)"

Write-Host "`nTest 1: Simple JSON"
& $wjq "tests\data\simple.json"
Write-Host "Exit code: $LASTEXITCODE"

Write-Host "`nDone!"
