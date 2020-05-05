@echo off

WHERE cl.exe
IF %ERRORLEVEL% NEQ 0 (
	ECHO cl.exe wasn't found.  Ensure that you've properly initialized the environment to include the MSVC compiler.
	exit
)

pushd ..\
if NOT EXIST ..\build (
    mkdir build
)
cd build
cmake -GNinja ../
popd