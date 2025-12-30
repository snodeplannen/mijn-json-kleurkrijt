$wjq = "..\build\wjq\Release\wjq.exe"
$testData = "tests\data"

function Run-Test {
    param($Name, $Args, $ExpectSuccess = $true)
    Write-Host "Running Test: $Name" -ForegroundColor Cyan
    $fullArgs = $Args
    $output = & $wjq $fullArgs 2>&1
    $exitCode = $LASTEXITCODE
    Write-Host $output
    if ($ExpectSuccess) {
        if ($exitCode -eq 0) {
            Write-Host "PASS: $Name" -ForegroundColor Green
        }
        else {
            Write-Host "FAIL: $Name (Expected success, got exit code $exitCode)" -ForegroundColor Red
        }
    }
    else {
        if ($exitCode -ne 0) {
            Write-Host "PASS: $Name (Expected failure, got exit code $exitCode)" -ForegroundColor Green
        }
        else {
            Write-Host "FAIL: $Name (Expected failure, got exit code 0)" -ForegroundColor Red
        }
    }
    Write-Host "-----------------------------------"
}

# Simple JSON
Run-Test "Simple JSON" "$testData\simple.json"

# Array JSON
Run-Test "Array JSON" "$testData\array.json"

# JSONL (Multiple docs)
Run-Test "JSONL (Multiple docs)" "$testData\mixed.jsonl"

# Theme test
Run-Test "Theme: Neon" "-t neon $testData\simple.json"

# Compact test
Run-Test "Compact output" "-c $testData\simple.json"

# Stdin test
Write-Host "Running Test: Stdin" -ForegroundColor Cyan
"{`"stdin`": true}" | & $wjq
if ($LASTEXITCODE -eq 0) {
    Write-Host "PASS: Stdin" -ForegroundColor Green
}
else {
    Write-Host "FAIL: Stdin (Exit code $LASTEXITCODE)" -ForegroundColor Red
}
Write-Host "-----------------------------------"

# Positional filter test
Run-Test "Positional Filter" ". $testData\simple.json"
