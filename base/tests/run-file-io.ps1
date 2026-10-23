param([string]$Clang)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products '*' -property installationPath
if (!$Clang) {
    $Clang = (Get-Command clang-cl.exe -ErrorAction SilentlyContinue).Source
    if (!$Clang) {
        $Clang = Get-ChildItem "$env:LOCALAPPDATA/Android/Sdk/ndk/*/toolchains/llvm/prebuilt/windows-x86_64/bin/clang-cl.exe" |
            Sort-Object FullName -Descending | Select-Object -First 1 -ExpandProperty FullName
    }
}
if (!$Clang -or !$vs) { throw 'Requires clang-cl and Visual Studio C++ build tools.' }
$out = Join-Path $repo 'paint/build/tests/file-io'
New-Item -ItemType Directory -Path $out -Force | Out-Null
$batch = @"
@echo off
call "$vs\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
"$Clang" /nologo /c /Od /Gy /Gw /D_CRT_SECURE_NO_WARNINGS /I"$repo\base\sources" "$PSScriptRoot\file_io.c" "$repo\base\sources\iron_string.c" "$repo\base\sources\iron_array.c" "$repo\base\sources\iron_map.c"
if errorlevel 1 exit /b 1
"%VCToolsInstallDir%bin\Hostx64\x64\link.exe" /nologo /OUT:file_io.exe /OPT:REF file_io.obj iron_string.obj iron_array.obj iron_map.obj kernel32.lib
if errorlevel 1 exit /b 1
"$Clang" /nologo /c /D_CRT_SECURE_NO_WARNINGS /I"$repo\base\sources" "$PSScriptRoot\windows_net.c"
if errorlevel 1 exit /b 1
"%VCToolsInstallDir%bin\Hostx64\x64\link.exe" /nologo /OUT:windows_net.exe windows_net.obj kernel32.lib winhttp.lib
"@
$batchPath = Join-Path $out 'compile.cmd'
Set-Content -LiteralPath $batchPath -Value $batch -Encoding ascii
Push-Location $out
try {
    $ErrorActionPreference = 'Continue' # Windows PowerShell otherwise treats compiler warnings on stderr as terminating errors.
    & $env:ComSpec /c $batchPath
    $ErrorActionPreference = 'Stop'
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
    $caseDir = Join-Path $out ('case-' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $caseDir | Out-Null
    & ./file_io.exe ($caseDir.Replace('\', '/'))
    if ($LASTEXITCODE -ne 0) { throw 'File I/O regression failed.' }
    & ./windows_net.exe
    if ($LASTEXITCODE -ne 0) { throw 'WinHTTP regression failed.' }
    if (Get-ChildItem -LiteralPath $caseDir -Filter '*.tmp') { throw 'Temporary files leaked.' }
}
finally { Pop-Location }
