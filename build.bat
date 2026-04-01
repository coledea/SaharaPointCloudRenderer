@echo off

goto MAIN

:build
    if not EXIST build\ mkdir build

    cd build

    cmake -DCMAKE_PREFIX_PATH="%QT_CMAKE%" -DCLANG_TOOLS_PATH="%CLANG_TOOLS%" -DBUILD_CLANG_FORMAT_TARGET="%BUILD_CLANG_FORMAT_TARGET%" -DENABLE_PROFILING="%ENABLE_PROFILING%" -DBYPASS_OS_FILE_CACHE="%BYPASS_OS_FILE_CACHE%" ../

    cmake --build . --config Debug
    cmake --build . --config Release

    cd ../

    exit /B 0

:clean
    rmdir /S /Q build

    exit /B 0

:rebuild
    call :clean
    call :build

    exit /B 0

:MAIN
    if "%~1"=="" (
        echo Please specify a build target. Main targets are build, rebuild and clean.
    ) else (
        call configure.bat
        call :%1%
    )