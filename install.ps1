$ErrorActionPreference='Stop'
$bundle=Join-Path $PSScriptRoot 'JOSEADOR.vst3'
$destination=Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3'
if(!(Test-Path -LiteralPath $bundle)){throw 'Falta la carpeta completa JOSEADOR.vst3'}
New-Item -ItemType Directory -Force $destination | Out-Null
$staging=Join-Path $destination ('JOSEADOR-staging-'+[Guid]::NewGuid().ToString('N'))
$allowed=[IO.Path]::GetFullPath($destination)+[IO.Path]::DirectorySeparatorChar
if(!([IO.Path]::GetFullPath($staging)).StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)){throw 'Ruta temporal fuera del directorio VST3'}
# Copy completely before replacing the installed bundle.
Copy-Item -LiteralPath $bundle -Destination $staging -Recurse -Force
$moved=@()
try {
$previous=Join-Path $destination 'WebCapture Mobile.vst3'
if(Test-Path -LiteralPath $previous){
 $resolved=[IO.Path]::GetFullPath($previous)
 $allowed=[IO.Path]::GetFullPath($destination)+[IO.Path]::DirectorySeparatorChar
 if(!$resolved.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)){throw 'Ruta anterior fuera del directorio VST3'}
 $backupRoot=Join-Path $env:LOCALAPPDATA 'WebCaptureMobile\PreviousVersions'
 New-Item -ItemType Directory -Force $backupRoot | Out-Null
 $backup=Join-Path $backupRoot ('WebCapture Mobile-'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'.vst3')
 Move-Item -LiteralPath $resolved -Destination $backup
 $moved+=@{Original=$resolved;Backup=$backup}
}
$current=Join-Path $destination 'JOSEADOR.vst3'
if(Test-Path -LiteralPath $current){
 $resolved=[IO.Path]::GetFullPath($current)
 $allowed=[IO.Path]::GetFullPath($destination)+[IO.Path]::DirectorySeparatorChar
 if(!$resolved.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)){throw 'Ruta de actualizacion fuera del directorio VST3'}
 $backupRoot=Join-Path $env:LOCALAPPDATA 'WebCaptureMobile\PreviousVersions'
 New-Item -ItemType Directory -Force $backupRoot | Out-Null
 $backup=Join-Path $backupRoot ('JOSEADOR-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff')+'.vst3')
 Move-Item -LiteralPath $resolved -Destination $backup
 $moved+=@{Original=$resolved;Backup=$backup}
}
Move-Item -LiteralPath $staging -Destination $current
} catch {
 for($i=$moved.Count-1;$i -ge 0;$i--){
  $item=$moved[$i]
  $original=[IO.Path]::GetFullPath($item.Original)
  $saved=[IO.Path]::GetFullPath($item.Backup)
  $backupAllowed=[IO.Path]::GetFullPath($backupRoot)+[IO.Path]::DirectorySeparatorChar
  if($original.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase) -and $saved.StartsWith($backupAllowed,[StringComparison]::OrdinalIgnoreCase) -and !(Test-Path -LiteralPath $original) -and (Test-Path -LiteralPath $saved)){Move-Item -LiteralPath $saved -Destination $original}
 }
 throw
}
Write-Host "Instalado: $destination\JOSEADOR.vst3"
Write-Host 'En FL Studio: Manage plugins > Find installed plugins; después añade JOSEADOR como instrumento. Creador: PAPIELGUAPO.'
