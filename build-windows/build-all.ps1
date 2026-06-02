# Cleans all the cmake files and the compiled binaries (debug version)

##Launch-VsDevShell.ps1 -SkipAutomaticLocation

Write-Host -ForegroundColor green "CLEANING BUILD FILES AND BINARIES ..."

Push-Location -Path .\cmake
Get-ChildItem -Force | Where-Object { $_.Name -ne '.gitignore' } | Remove-Item -Recurse -Force
Pop-Location 
Remove-Item -Force -Recurse bin/Debug*, bin/Release*
Write-Host -ForegroundColor green "Done."



Push-Location -Path .\cmake
Write-Host -ForegroundColor green "Creating build files ... "
cmake .. 
Write-Host -ForegroundColor green "Done."
Write-Host -ForegroundColor green "Compiling the code ..."
cmake --build . --config Debug
Pop-Location
Write-Host -ForegroundColor green "Done."

