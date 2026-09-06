param(
    [Parameter(Mandatory=$true)][string]$CodeFile,
    [string]$Uri = 'http://127.0.0.1:49620/execute'
)
$code = Get-Content -Raw -LiteralPath $CodeFile
$body = @{ code = $code } | ConvertTo-Json -Compress
$tmp = Join-Path $env:TEMP ('eda_body_' + [guid]::NewGuid().ToString('N') + '.json')
[IO.File]::WriteAllText($tmp, $body, (New-Object System.Text.UTF8Encoding($false)))
try {
    $raw = & curl.exe -s -X POST $Uri -H 'Content-Type: application/json' --data-binary "@$tmp"
    $raw
} finally {
    Remove-Item -LiteralPath $tmp -Force -ErrorAction SilentlyContinue
}
