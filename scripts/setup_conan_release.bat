@echo off

pushd ..
call conan install . --build=missing --settings=build_type=Release
call .\vendor\premake\premake5.exe vs2022
popd

pause