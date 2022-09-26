#Create a shortcut in the startup folder to run ConfettiCannon at boot

Write-Host "Warning!!!!! This script may produce a non-working shortcut until fixes are determined. Please test the shortcut manually after running to verify."

$ShellFolders = (Get-ItemProperty 'HKCU:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders')

$Shelltime = New-Object -comObject WScript.Shell
$ShortcutPath = ($ShellFolders.Startup + "\ConfettiCannon.exe - Shortcut.lnk")
#$ShortcutPath = ($pwd.Path + "\ConfettiCannon.exe - shortcut.lnk")

Write-Host ("Confetti launcher shortcut will be placed at: " + $ShortcutPath)

$Shortcut = $Shelltime.CreateShortcut($ShortcutPath)

$ExeLocation = ($pwd.Path + "\ConfettiCannon.exe")


if (-not (Test-Path $ExeLocation)){
	Write-Host "Error: Could not find ConfettiCannon.exe. Please ensure this script is placed in the same directory as ConfettiCannon.exe!"
	Exit
}

$Shortcut.TargetPath = ($ExeLocation)
$Shortcut.Save()

#Move-Item $ShortcutPath $ShellFolders.Startup

if (-not (Test-Path $ShortcutPath)){
	Write-Host "Error: ConfettiCannon.exe was successfully located, but shortcut generation failed"
	Exit
}
else{
	Write-Host "Success!"
}