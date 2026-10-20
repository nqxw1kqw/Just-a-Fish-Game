# 日本語専用の配布用フォルダーと ZIP を作成します。元の素材は変更しません。
param()
$ErrorActionPreference = 'Stop'
$previousOutputEncoding = [Console]::OutputEncoding
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
$taskRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
Push-Location -LiteralPath $taskRoot
try {
    $exe = Join-Path $taskRoot 'FishGame.exe'
    if (!(Test-Path -LiteralPath $exe)) { throw '先に Release x64 をビルドしてください。' }
    $version = Get-Date -Format 'yyyyMMdd-HHmmss'
    $packageRoot = Join-Path $taskRoot ('obj\Packages\Japanese-' + $version)
    if (Test-Path -LiteralPath $packageRoot) { throw '同名の出力先が存在します。' }
    $package = Join-Path $packageRoot 'Just-a-Fish-Game'
    New-Item -ItemType Directory -Path $package | Out-Null
    Copy-Item -LiteralPath $exe -Destination (Join-Path $package 'FishGame.exe')

    $files = @(& git -c core.quotepath=false ls-files -- Data)
    if ($LASTEXITCODE -ne 0) { throw 'Git の追跡ファイルを取得できません。' }
    # 新しい日本語画像は、コミット前でも配布用へ含めます。
    $files += @('Data/UI/Pause/Layered-v1/row-music-ja.png',
                'Data/UI/Pause/Layered-v1/row-sfx-ja.png')
    $oldNames = @('header-language.png','language-english.png','language-japanese.png',
                  'language.png','Yabikoma-e9W16.otf','confirm.png','row-bgm.png','row-sfx.png')
    $excludedCount = 0
    [long]$excludedBytes = 0
    foreach ($relative in ($files | Sort-Object -Unique)) {
        $leaf = [IO.Path]::GetFileName($relative)
        $source = Join-Path $taskRoot $relative
        if ($leaf -like '*-en.png' -or $relative -like 'Data/UI/LanguageSelect/*' -or $leaf -in $oldNames -or $relative -like 'Data/UI/ModeSelect/*' -or ($relative -like 'Data/UI/ChoiceMenus/*' -and $relative -ne 'Data/UI/ChoiceMenus/Layered-v1/background.png')) {
            $excludedCount++
            $excludedBytes += (Get-Item -LiteralPath $source).Length
            continue
        }
        # 記録や個人設定は配布しません。
        if ($leaf -like 'highscore*' -or $leaf -eq 'settings.ini') { continue }
        $destination = Join-Path $package $relative
        New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
        Copy-Item -LiteralPath $source -Destination $destination
    }
    @'
日本語専用版

FishGame.exe を起動してください。Data フォルダーは実行ファイルと同じ場所に置いてください。
スペースまたはエンターで開始し、W/A/S/D または矢印キーで移動します。
スペースでジャンプし、着地時に攻撃します。Esc/P で一時停止します。
日本語専用です。チュートリアル後は25秒のウェーブが3回続き、その後にボスと戦います。
最初のボスは必ずクジラです。合計5体のボスを倒すと勝利します。
難易度選択・英語用の素材と個人の記録はこの配布物に含まれていません。
'@ | Set-Content -LiteralPath (Join-Path $package 'README.txt') -Encoding utf8
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = Join-Path $packageRoot 'Just-a-Fish-Game-Japanese.zip'
    [IO.Compression.ZipFile]::CreateFromDirectory($package, $zip, [IO.Compression.CompressionLevel]::Optimal, $false)
    Write-Output ('除外した素材: {0} ファイル / {1:N2} MiB' -f $excludedCount, ($excludedBytes / 1MB))
    Write-Output ('配布用 ZIP: ' + $zip)
}
finally { Pop-Location; [Console]::OutputEncoding = $previousOutputEncoding }
