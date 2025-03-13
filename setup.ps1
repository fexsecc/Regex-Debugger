# Get the directory of the script and cd into it
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location -Path $scriptDir

# Clone vcpkg and run cmake with preset
git clone https://github.com/microsoft/vcpkg.git vcpkg
cmake --preset=default-msvc
