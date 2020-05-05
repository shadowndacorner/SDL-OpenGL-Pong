@echo off

call build.bat

IF %ERRORLEVEL% NEQ 0 (
    echo Build failed
    popd
    exit
)


cd ..\bin
pong.exe
popd