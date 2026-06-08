# run_coverage.ps1 — 유닛테스트 + 전체 TestScripts 커버리지 측정
param(
    [string]$ExePath   = "x64\Debug\CodeFab_Interpreter.exe",
    [string]$OutDir    = "coverage_report",
    [string]$SourceDir = "CodeFab_Interpreter"
)

$OCP = "C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe"
$CovDir = "coverage_tmp"
New-Item -ItemType Directory -Force -Path $CovDir | Out-Null
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$unitCov = "$CovDir\unit.cov"

Write-Host "=== [1/3] 유닛 테스트 커버리지 측정 ===" -ForegroundColor Cyan
& $OCP `
    --sources $SourceDir `
    --export_type "binary:$unitCov" `
    --quiet `
    -- $ExePath unit 2>&1 | Where-Object { $_ -notmatch "^$" }

if (-not (Test-Path $unitCov)) {
    Write-Error "유닛 테스트 커버리지 파일 생성 실패"
    exit 1
}
Write-Host "  -> $unitCov 생성 완료" -ForegroundColor Green

Write-Host "`n=== [2/3] TestScripts 커버리지 누적 ===" -ForegroundColor Cyan
$scripts = Get-ChildItem -Path "TestScripts" -Filter "*.txt" -Recurse | Sort-Object FullName
$prevCov = $unitCov
$count   = 0

foreach ($script in $scripts) {
    $count++
    $outCov = "$CovDir\acc_$count.cov"
    Write-Host "  [$count/$($scripts.Count)] $($script.Name)" -NoNewline
    & $OCP `
        --sources $SourceDir `
        --input_coverage $prevCov `
        --export_type "binary:$outCov" `
        --quiet `
        -- $ExePath run $script.FullName 2>&1 | Out-Null
    if (Test-Path $outCov) {
        Write-Host "  OK" -ForegroundColor Green
        $prevCov = $outCov
    } else {
        Write-Host "  SKIP (cov 생성 실패)" -ForegroundColor Yellow
    }
}

Write-Host "`n=== [3/3] HTML 리포트 생성 ===" -ForegroundColor Cyan
$htmlOut = "$OutDir\index.html"
& $OCP `
    --sources $SourceDir `
    --input_coverage $prevCov `
    --export_type "html:$OutDir" `
    --quiet 2>&1 | Out-Null

if (Test-Path $htmlOut) {
    Write-Host "  -> 리포트: $((Resolve-Path $htmlOut).Path)" -ForegroundColor Green
} else {
    Write-Host "  -> HTML 생성 실패" -ForegroundColor Red
}

Remove-Item -Recurse -Force $CovDir -ErrorAction SilentlyContinue
Write-Host "`n완료. 브라우저에서 $OutDir\index.html 을 열어주세요." -ForegroundColor Cyan
