@echo off

WHERE cl.exe
IF %ERRORLEVEL% NEQ 0 (
	ECHO cl.exe wasn't found.  Ensure that you've properly initialized the environment to include the MSVC compiler.
	exit
)

if NOT EXIST ..\build (
    call cmake.bat
)
    
pushd ..\build
cmake --build .
popd

if NOT EXIST ..\bin\data (
	mkdir ..\bin\data
)

robocopy ..\data ..\bin\data /e /MT /NDL /NJH /NJS /nc /ns /PURGE