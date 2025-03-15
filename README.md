# Regex-Debugger
## Building
### Windows

```powershell
git clone https://github.com/microsoft/vcpkg.git
Set-ExecutionPolicy Bypass -Scope Process # enable running the setup ps1 script
.\setup.ps1
cmake --build build
.\vsSetup.ps1 #Removes ALL_BUILD and ZERO_CHECK from the vs sln
```
### Linux/Mac
```bash
./setup.sh
cmake --build build
```
