# README ![image](./data/icons/sahara.png)  
This branch contains the version of the Sahara Point Cloud Renderer used for evaluating the approaches presented in our paper:

**Quantitative Evaluation of Comparative Visualization Methods for Change Identification in Multi-Temporal Point Clouds**, Wegen et al., 2026.

For the most recent version of the renderer, please use the [`main`](../../) branch. It omits several paper-specific features, such as the user-study overlay, and reflects the current state of development.
Additional supplementary material is available at [Zenodo](https://zenodo.org/records/21379345).


## Setup Instructions
To setup the renderer, follow these steps:

1. **Install dependencies**
    - [CMake](https://cmake.org) (minimum version 4.0)
    - [Qt](https://www.qt.io/download-dev) (minimum version 6.7).

2. **Build the project using CMake**
    - On Windows and Linux, you can use the respective build script:
        - For Windows, create a `configure.bat` file by copying `configure_template.bat` and fill in the correct values. Then run `build.bat build`.
        - For Linux, create a `configure.sh` file by copying `configure_template.sh` and fill in the correct values. Then run `build.sh build`.
    - To enable the user study overlay, `ENABLE_USER_STUDY_MODE=ON` has to be set in the `configure` script.

3. **Run the application**
    - Either launch from within your IDE
    - Or build the INSTALL target to create a folder containing the executable along with all required dependencies


## Usage
The point clouds used in the user study are available at [Zenodo](https://zenodo.org/records/21379345).
Just download the datasets.zip and unpack it.

A multi-temporal point cloud can be opened via the _File_ menu by selecting an _.mtpc_ file.
In the _Postprocessing_ tab on the right side of the user interface, eye-dome lighting can be enabled.
In the _Rasterizer_ tab, rendering parameters such as the visualization method and point size can be configured.

## Interaction
Navigation is performed using the mouse:
 - **Left mouse button:** pan
 - **Right mouse button:** rotate
 - **Mouse wheel:** zoom


Further Interaction:
 - **Split view:** Hold CTRL and drag the split line with the mouse to move it.
 - **Difference highlighting:** Hold CTRL to open the interactive lens. While the lens is open, press the left mouse button to switch the timestamp shown inside the lens.
 - **Annotation:** Press SPACE to open the annotation overlay. While the overlay is active, annotations can be added by clicking on a pixel in the rendered image.



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
