# Cleans all the cmake files and the compiled binaries

##Launch-VsDevShell.ps1 -SkipAutomaticLocation

Write-Host -ForegroundColor green "CLEANING BUILD FILES AND BINARIES ..."

Push-Location -Path .\cmake
Get-ChildItem -Force | Where-Object { $_.Name -ne '.gitignore' } | Remove-Item -Recurse -Force
Pop-Location 
Remove-Item -Force -Recurse bin/Debug*, bin/Release*
Write-Host -ForegroundColor green "Done."

