$target = "C:\Users\hyper\workspace\bobfilez\tests\test_cmake_build\subdirectory_function\build_output"
$empty = "C:\Users\hyper\workspace\bobfilez\tests\_empty_temp"

New-Item -ItemType Directory -Path $empty -Force | Out-Null
robocopy $empty $target /MIR /R:1 /W:1 /NJH /NJS | Out-Null
Remove-Item -Recurse -Force $empty -ErrorAction SilentlyContinue
Write-Host "pybind11 recursive directory cleaned"
