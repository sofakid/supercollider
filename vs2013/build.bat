
set QT_HOME=C:\Qt\5.4

set CMAKE_PREFIX_PATH=%QT_HOME%\msvc2013_64
set CMAKE_LIBRARY_ARCHITECTURE=x64

mkdir build
cd build

cmake -G "Visual Studio 12 2013 Win64" ..\..\.

msbuild /m:6 /target:ALL_BUILD;INSTALL Supercollider.sln

start Supercollider.sln

cd ..
