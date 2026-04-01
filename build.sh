if [ ! -f "configure.sh" ]; then
    echo "Configuration script (configure.sh) not found! Please copy from configure_template.sh and adapt it."
    exit 1
fi

source configure.sh

if [ ! -d "build" ]; then
    mkdir build
fi
cd build

if [ "$1" = "clean" ] || [ "$1" = "rebuild" ]; then
    rm -rf *
    echo "Cleaned build directory."
fi

if [ $# -eq 0 ] || [ "$1" = "build" ] || [ "$1" = "rebuild" ]; then
    cmake -DCMAKE_PREFIX_PATH="$QT_CMAKE" ../ -DCLANG_TOOLS_PATH="${CLANG_TOOLS}" -DBUILD_CLANG_FORMAT_TARGET="${BUILD_CLANG_FORMAT_TARGET}" -DENABLE_PROFILING="${ENABLE_PROFILING}" -DBYPASS_OS_FILE_CACHE="${BYPASS_OS_FILE_CACHE}"

    cmake --build . --config Debug
    cmake --build . --config Release
fi

cd ../