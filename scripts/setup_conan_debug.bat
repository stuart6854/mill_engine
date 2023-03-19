@echo off

pushd ..
call conan install . --build=missing --settings=build_type=Debug
popd

pause