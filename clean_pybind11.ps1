$path = "C:\Users\hyper\workspace\bobfilez\tests\test_cmake_build\subdirectory_function\build_output\pybind11"
$longPath = "\\?\$path"
if (Test-Path $longPath) {
    Write-Output "Found pybind11, removing..."
    Remove-Item -LiteralPath $longPath -Recurse -Force -ErrorAction Stop
    Write-Output "Removed successfully"
} else {
    Write-Output "pybind11 not found at path"
    # Try without long path prefix
    if (Test-Path $path) {
        Write-Output "Found without prefix, removing..."
        Remove-Item -LiteralPath $path -Recurse -Force -ErrorAction Stop
        Write-Output "Removed"
    }
}
