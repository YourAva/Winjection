:: you dont need to understand this, its js for simplicity

@echo off
set SRC_DIR=src
set BUILD_DIR=build

gcc -c -o %BUILD_DIR%\tempDLL.o %SRC_DIR%\dll.c -D ADD_EXPORTS
gcc -o %BUILD_DIR%\ALB.dll %BUILD_DIR%\tempDLL.o -s -shared -Wl,--subsystem,windows
del %BUILD_DIR%\tempDLL.o


if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
)

echo Compilation successful.