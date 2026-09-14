# README ![image](./data/icons/sahara.png)  
This repository contains the Sahara Point Cloud Renderer which has been primarily developed for research purposes.
It includes implementations of the approaches presented in our papers:

- [**Out-of-Core Rendering of Multi-Temporal Point Clouds**](https://doi.org/10.2312/egpgv.20261002), Wegen et al., 2026, EGPGV.
- **Quantitative Evaluation of Comparative Visualization Methods for Change Identification in Multi-Temporal Point Clouds**, Wegen et al., 2026, VINCI.

The `main` branch contains the most recent version of the renderer. It omits several paper-specific features, such as benchmarking or user study functionality.
The specific code versions used for the corresponding evaluations are preserved in the [`egpgv_2026`](../../tree/egpgv_2026) and [`vinci_2026`](../../tree/vinci_2026) branches and as dedicated releases.


## Setup Instructions
The renderer is written in C++ and uses Qt and OpenGL.
To setup the renderer, follow these steps:

1. **Install dependencies**
    - [CMake](https://cmake.org) (minimum version 4.0)
    - [Qt](https://www.qt.io/download-dev) (minimum version 6.7).

2. **Build the project using CMake**
    - On Windows and Linux, you can use the respective build script:
        - For Windows, create a `configure.bat` file by copying `configure_template.bat` and fill in the correct values. Then run `build.bat build`.
        - For Linux, create a `configure.sh` file by copying `configure_template.sh` and fill in the correct values. Then run `build.sh build`.
    - To enable profiling, `PROFILER_ENABLED=ON` has to be set in the `configure` script. If the OS file cache should be bypassed to avoid caching effects for consecutive measurements `BYPASS_OS_FILE_CACHE=ON` has to be set. Both options activate corresponding preprocessor defines.

3. **Run the application**
    - Either launch from within your IDE
    - Or build the INSTALL target to create a folder containing the executable along with all required dependencies



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
   Note that this disables parallel execution of standard-library algorithms in those compilation units.
   `configure_template.sh` already contains a corresponding flag that can simply be enabled.


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

```
@inproceedings{wegen2026multitemporalVisualization,
    author = {Wegen, Ole and Heidisch, Lukas and Scheibel, Willy and  Richter, Rico and Barz-Cech, Tim and Döllner, Jürgen},
    title = {Quantitative Evaluation of Comparative Visualization Methods for Change Identification in Multi-Temporal Point Clouds},
    booktitle = {Proceedings of the 19th International Symposium on Visual Information Communication and Interaction (VINCI)},
    publisher = {ACM},
    year = {2026},
    doi = {TBD}
}
```

## Acknowledgements
This renderer was primarily developed by Ole Wegen. Special thanks go to Matthias Trapp for his work on the initial version of the renderer, from which several UI components and the icon design were retained.
Additional thanks go to Andreas Franke, Jorge Ciprián-Sánchez, Lukas Heidisch, Sandro Steeger, and Ulrike Herwig, who contributed to various parts of the system, such as I/O functionality, specific rasterization and visualization approaches, architectural improvements, and bug fixes.

## License
This project is licensed under the MIT License (see the LICENSE.md file). Portions of this project incorporate [tinyply](https://github.com/ddiakopoulos/tinyply), which is released into the public domain (with fallback licensing as described in its source).