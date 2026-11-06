param([string]$Configuration='Release')
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
$deps=Join-Path $root '.deps'
New-Item -ItemType Directory -Force $deps | Out-Null
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(!$vs){throw 'Instala Visual Studio con herramientas C++ x64, CMake y Windows 11 SDK.'}
$cmake=Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
if(!(Test-Path "$deps\JUCE-8.0.6")){
 Invoke-WebRequest 'https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.6.zip' -OutFile "$deps\juce.zip"
 Expand-Archive "$deps\juce.zip" $deps -Force
}
$cefName='cef_binary_150.0.10+g8042e43+chromium-150.0.7871.101_windows64_minimal'
if(!(Test-Path "$deps\$cefName")){
 $url='https://github.com/psmux/godwinmix-cef/releases/download/cef-150.0.10/'+[Uri]::EscapeDataString($cefName+'.tar.bz2')
 Invoke-WebRequest $url -OutFile "$deps\cef.tar.bz2"
 if((Get-FileHash "$deps\cef.tar.bz2" -Algorithm SHA256).Hash -ne '01ee170162e576beeb5c0d8de1c2c92c5d2f5f2ce9d47e14825acc8eae873dc2'){throw 'La suma SHA-256 de CEF no coincide.'}
 & tar -xf "$deps\cef.tar.bz2" -C $deps
 if($LASTEXITCODE){throw 'Error extrayendo CEF'}
}
$previousCompilerOptions=$env:_CL_
try{
 $env:_CL_='/Z7 /MP4'
 & $cmake -S $root -B "$root\build" -A x64 "-DJUCE_DIR:PATH=$deps/JUCE-8.0.6" "-DCEF_ROOT:PATH=$deps/$cefName" -DWEBCAPTURE_BUILD_TESTS=OFF
 if($LASTEXITCODE){throw 'Error configurando CMake'}
 & $cmake --build "$root\build" --config $Configuration --target WebCaptureMobile_VST3 MobileBrowser JOSEADORInstaller --parallel 4
 if($LASTEXITCODE){throw 'Error compilando'}
 $built=Join-Path $root "build\WebCaptureMobile_artefacts\$Configuration\VST3\JOSEADOR.vst3"
 $runtime=Join-Path $built 'Contents\x86_64-win\BrowserRuntime'
 New-Item -ItemType Directory -Force $runtime | Out-Null
 Copy-Item "$root\build\BrowserRuntime\$Configuration\*" $runtime -Recurse -Force
 $package=Join-Path $root 'package'
 New-Item -ItemType Directory -Force $package | Out-Null
 $licenses=Join-Path $built 'Contents\Resources\Licenses'
 New-Item -ItemType Directory -Force $licenses | Out-Null
 Copy-Item "$root\ThirdParty\*" $licenses -Recurse -Force
 Copy-Item -LiteralPath $built -Destination $package -Recurse -Force
 Copy-Item "$root\build\$Configuration\Instalar JOSEADOR.exe" $package -Force
 & "$root\Installer\prepare-redistributable.ps1" -Destination "$package\Prerequisites"
 Write-Host "VST3 completo: $built"
 Write-Host "Paquete con instalador EXE y prerequisitos: $package"
}finally{$env:_CL_=$previousCompilerOptions}
