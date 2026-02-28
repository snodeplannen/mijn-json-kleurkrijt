$wjq = ".\build\wjq\Release\wjq.exe"
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
$themes = @("default", "dracula", "solarized", "monokai", "github", "minimal", "neon",
    "ocean", "forest", "cyberpunk", "sunset", "high-contrast", "white",
    "nord", "gruvbox", "one-dark", "catppuccin", "ice", "coffee")

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

Show-Header "Value-gebaseerde Kleuring (boolean true=groen, false=rood)"
Write-Host "Booleans en null waarden:" -ForegroundColor Yellow
'{
  "feature_flags": {
    "dark_mode": true,
    "beta_access": false,
    "notifications": true,
    "maintenance_mode": false,
    "api_v2": true
  },
  "user": {
    "name": "Alice",
    "verified": true,
    "banned": false,
    "deleted_at": null,
    "score": 42.5
  }
}' | & $wjq

Show-Header "Debug Thema (error/warning highlighting)"
Write-Host "Het debug thema kleurt strings met 'error', 'warning', 'fail' etc. anders:" -ForegroundColor Yellow
'{
  "status": "error",
  "level": "warning",
  "message": "Connection failed",
  "retry": true,
  "fallback": null,
  "details": {
    "code": 500,
    "type": "critical",
    "resolved": false,
    "info": "success"
  }
}' | & $wjq -t debug

Show-Header "Depth-Aware Thema (diepte-gebaseerde kleuring)"
Write-Host "Kleuren veranderen op basis van nesting-diepte:" -ForegroundColor Yellow
'{
  "level_0": {
    "level_1": {
      "level_2": {
        "level_3": {
          "deep": true
        }
      },
      "sibling": "waarde"
    },
    "items": [1, 2, 3]
  }
}' | & $wjq -t depth-aware

Show-Header "Einde Demo"
