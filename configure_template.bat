@echo off

:: The cmake folder of your QT installation. Can be left empty, if the folder is in PATH.
set QT_CMAKE=C:\Qt\6.9.3\msvc2022_64\lib\cmake

:: Whether to build a target that runs clang-format on the code. If the target should be build, CLANG_TOOLS has to be set to the folder containing the clang-format executable.
set BUILD_CLANG_FORMAT_TARGET=OFF
set CLANG_TOOLS=PATH_TO_CLANG_TOOLS_FOLDER

:: ENABLE_PROFILING=ON activates the code paths used for logging certain metrics during rendering and memory management. Switching this OFF might slightly improve the performance.
set ENABLE_PROFILING=OFF

:: If set to ON, point data is streamed in out-of-core rendering directly from external storage. This can be used to avoid caching effects during profiling, where later measurements perform better due to OS file caching.
set BYPASS_OS_FILE_CACHE=OFF


:: NO NEED TO CHANGE ANYTHING BELOW THIS LINE
:: ==========================================

set PATH=%QT_CMAKE%;%PATH%


