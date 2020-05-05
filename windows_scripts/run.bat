@echo off

WHERE cl.exe
IF %ERRORLEVEL% NEQ 0 (
	ECHO cl.exe wasn't found.  Ensure that you've properly initialized the environment to include the MSVC compiler.
	exit
)

if NOT EXIST ..\build (
    pushd ..\
    mkdir build
    cd build
    cmake -GNinja ../
    popd
)

pushd ..\build
cmake --build .

IF %ERRORLEVEL% NEQ 0 (
    echo Build failed
    popd
    exit
)


cd ..\bin
pong.exe
popd