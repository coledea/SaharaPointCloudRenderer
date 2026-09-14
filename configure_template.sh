# The cmake folder of your QT installation.
export QT_CMAKE="~/Qt/6.9.3/gcc_64/lib/cmake"
export ENABLE_USER_STUDY_MODE=ON

# Whether to build a target that runs clang-format on the code. If the target should be build, CLANG_TOOLS has to be set to the folder containing the clang-format executable.
export BUILD_CLANG_FORMAT_TARGET=OFF
export CLANG_TOOLS="/usr/bin/" # ensure that the program `clang-format` exists. 

# ENABLE_PROFILING=ON activates the code paths used for logging certain metrics during rendering and memory management. Switching this OFF might slightly improve the performance.
export ENABLE_PROFILING=OFF

# If set to ON, point data is streamed in out-of-core rendering directly from external storage. This can be used to avoid caching effects during profiling, where later measurements perform better due to OS file caching.
export BYPASS_OS_FILE_CACHE=OFF

# If libstdc++'s TBB backend conflicts with Qt, activate this flag to prevent TBB being used. However, this will disable parallel algorithm execution.
export DEACTIVATE_TBB=OFF


