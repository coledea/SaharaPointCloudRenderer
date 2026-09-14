# README ![image](./data/icons/sahara.png)
This repository contains the Sahara Point Cloud Renderer which has been primarily developed for research purposes.
It includes an implementation of the rendering approach presented in our paper:

**Out-of-Core Rendering of Multi-Temporal Point Clouds**, Wegen et al., 2026.

The renderer is written in C++ and uses Qt and OpenGL.

## Setup Instructions
To setup the renderer, follow these steps:

1. **Install dependencies**
    - [CMake](https://cmake.org) (minimum version 3.16; 4.x version probably won't work)
    - [Qt](https://www.qt.io/download-dev) (minimum version 6.7).

2. **Build the project using CMake**
    - On Windows and Linux systems, you can use the respective build script:
        - For Windows, create a `configure.bat` file by copying `configure_template.bat` and fill in the correct values. Then run `build.bat build`.
        - For Linux, create a `configure.sh` file by copying `configure_template.sh` and fill in the correct values. Then run `build.sh build`.
    - To enable profiling, `PROFILER_ENABLED=ON` has to be set in the `configure` script. If the OS file cache should be bypassed to avoid caching effects for consecutive measurements `BYPASS_OS_FILE_CACHE=ON` has to be set. Both options activate corresponding preprocessor defines.

3. **Run the application**
    - Either launch from within your IDE
    - Or build the INSTALL target to create a folder containing the executable along with all required dependencies

To reproduce the experiments described in the paper, refer to [EGPGV_2026.md](./instructions/EGPGV_2026.md).

## Known Issues
### Wayland on Fedora
When running under Wayland on Fedora, a bug may cause the framebuffer to be created without a valid extent. As a workaround, force Qt to use the X11 backend instead by setting the enviroment variable `QT_QPA_PLATFORM=xcb`.
### oneTBB and Qt Conflicting
Sahara uses the `<execution>` header in some places for parallel processing. In the GNU libstdc++, these algorithms are implemented using oneTBB ([https://github.com/gcc-mirror/gcc/blob/8be0893fd98c9a89bbcd81e0ff8ebae60841d062/libstdc%2B%2B-v3/include/bits/c%2B%2Bconfig#L948](https://github.com/gcc-mirror/gcc/blob/8be0893fd98c9a89bbcd81e0ff8ebae60841d062/libstdc%2B%2B-v3/include/bits/c%2B%2Bconfig#L948)). 
However, oneTBB conflicts with QT due to name collisions (e.g., with respect to the `emit` keyword: [https://github.com/uxlfoundation/oneTBB/issues/547](https://github.com/uxlfoundation/oneTBB/issues/547)), leading to compiler errors when both are used together.
If both oneTBB and libstdc++ with parallel algorithm support are present on your system, there are two options for building Sahara:

1. **Disable Qt keyword macros**
   Define `QT_NO_KEYWORDS` and replace all Qt-specific macros (`emit`, `signals`, `slots`) with their explicit equivalents (`Q_EMIT`, `Q_SIGNALS`, `Q_SLOTS`).
   This approach preserves full support for parallel algorithms.

2. **Disable the TBB parallel backend locally**
   Define `_GLIBCXX_USE_TBB_PAR_BACKEND` in the affected compilations units to prevent libstdc++ from using oneTBB.
   Note that this disables parallel algorithm execution in those compilation units.


## Citation
```
@inproceedings{wegen2026,
    author = {Wegen, Ole and Steeger, Sandro and Scheibel, Willy and  Richter, Rico and Döllner, Jürgen},
    title = {Out-of-Core Rendering of Multi-Temporal Point Clouds},
    booktitle = {Proceedings of the 26th Eurographics Symposium on Parallel Graphics and Visualization (EGPGV)},
    year = {2026},
    doi = {10.2312/egpgv.20261002}
}
```

## Acknowledgements
This renderer was primarily developed by Ole Wegen. Special thanks go to Matthias Trapp for his work on the initial version of the renderer, from which several UI components and the icon design were retained; to Sandro Steeger and Andreas Franke for Linux-specific fixes; and to Jorge Ciprián-Sánchez for his contributions to PLY loading functionality.


## License
This project is licensed under the MIT License (see the LICENSE.md file). Portions of this project incorporate [tinyply](https://github.com/ddiakopoulos/tinyply), which is released into the public domain (with fallback licensing as described in its source).
