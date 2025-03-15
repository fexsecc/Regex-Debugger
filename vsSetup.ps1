#go into the build directory
cd build

dotnet sln regdebug.sln remove ALL_BUILD.vcxproj
dotnet sln regdebug.sln remove ZERO_CHECK.vcxproj

#cd to the original directory
cd ..