@echo off

pushd ..
call conan install . --build=missing --settings=build_type=Release
popd

pause