@echo off

if NOT EXIST ..\bin\pong.exe (
    echo Executable does not exist
    exit
)

pushd ..\bin
    devenv pong.exe
popd