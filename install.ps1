$ErrorActionPreference='Stop'
$bundle=Join-Path $PSScriptRoot 'Joseador.vst3'
$destination=Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3'
if(!(Test-Path -LiteralPath $bundle)){throw 'Falta la carpeta completa Joseador.vst3'}
New-Item -ItemType Directory -Force $destination | Out-Null
$staging=Join-Path $destination ('Joseador-staging-'+[Guid]::NewGuid().ToString('N'))
$allowed=[IO.Path]::GetFullPath($destination)+[IO.Path]::DirectorySeparatorChar
if(!([IO.Path]::GetFullPath($staging)).StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)){throw 'Ruta temporal fuera del directorio VST3'}
# Copy completely before replacing the installed bundle.
Copy-Item -LiteralPath $bundle -Destination $staging -Recurse -Force
$previous=Join-Path $destination 'WebCapture Mobile.vst3'
if(Test-Path -LiteralPath $previous){
 $resolved=[IO.Path]::GetFullPath($previous)
 $allowed=[IO.Path]::GetFullPath($destination)+[IO.Path]::DirectorySeparatorChar
 if(!$resolved.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)){throw 'Ruta anterior fuera del directorio VST3'}
 $backupRoot=Join-Path $env:LOCALAPPDATA 'WebCaptureMobile\PreviousVersions'
 New-Item -ItemType Directory -Force $backupRoot | Out-Null
 $backup=Join-Path $backupRoot ('WebCapture Mobile-'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'.vst3')
 Move-Item -LiteralPath $resolved -Destination $backup
}
$current=Join-Path $destination 'Joseador.vst3'
if(Test-Path -LiteralPath $current){
 $resolved=[IO.Path]::GetFullPath($current)
 $allowed=[IO.Path]::GetFullPath($destination)+[IO.Path]::DirectorySeparatorChar
 if(!$resolved.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)){throw 'Ruta de actualizacion fuera del directorio VST3'}
 $backupRoot=Join-Path $env:LOCALAPPDATA 'WebCaptureMobile\PreviousVersions'
 New-Item -ItemType Directory -Force $backupRoot | Out-Null
 $backup=Join-Path $backupRoot ('Joseador-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff')+'.vst3')
 Move-Item -LiteralPath $resolved -Destination $backup
}
Move-Item -LiteralPath $staging -Destination $current
Write-Host "Instalado: $destination\Joseador.vst3"
Write-Host 'En FL Studio: Manage plugins > Find installed plugins; después añade Joseador como instrumento. Creador: PAPIELGUAPO.'
