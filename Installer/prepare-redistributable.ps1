param([Parameter(Mandatory=$true)][string]$Destination)
$ErrorActionPreference='Stop'
New-Item -ItemType Directory -Force $Destination | Out-Null
$file=Join-Path $Destination 'VC_redist.x64.exe'
if(!(Test-Path -LiteralPath $file)){Invoke-WebRequest 'https://aka.ms/vc14/vc_redist.x64.exe' -OutFile $file}
$signature=Get-AuthenticodeSignature -LiteralPath $file
if($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'Microsoft Corporation'){throw 'La firma del redistribuible no es valida o no pertenece a Microsoft.'}
if([version](Get-Item -LiteralPath $file).VersionInfo.FileVersion -lt [version]'14.51.36231.0'){throw 'El runtime de Microsoft debe ser 14.51.36231 o posterior.'}
Write-Host 'Runtime oficial de Microsoft verificado. No se ha instalado durante la compilacion.'
