# EGPGV 2026 Paper

The point clouds used in the evaluation, already converted to our out-of-core format, are available at: [https://storage-share.pointcloudtechnology.com/s/9pyXNWAKpk7yF22](https://storage-share.pointcloudtechnology.com/s/9pyXNWAKpk7yF22) (password: 7spj87kxa6).
For the Noordwijk dataset, the *point.bin* is split into multiple .tar archives (due to its large size) and must be extracted before use.

![image](./data/ui_screenshot.png)
_Screenshot of the renderer_

## Reproducing the Experiments for Temporal Exploration
1. **Load point cloud and set parameters**
    - Open a point cloud via the _File_ menu (highlighted in green in the screenshot).
    - Set the correct parameter values in the parameter panel (highlighted in orange)
        - *Projected Size Pixel Threshold*: 250 for Hessigheim, 150 for the others
        - *Benchmarking Mode*: Timestamp Switches
        - *Benchmark Num Samples*: 600
        - *Timestamp Duration*: Depends on the point cloud and storage device. For point clouds with larger timestamps and for slow storage devices, higher values should be selected. We used values between 0.5s and 8s. The goal is to provide enough time for loading all chunks selected for rendering, in order to be able to measure the time-to-frame-completion.
    - Select the correct *Benchmark Camera Positions* for the dataset. The camera paths used in our evaluation are located in the `camera_paths/temporal_experiments` folder.
    - Add the hole-filling postprocessing step via the + icon in the postprocessing tab (highlighted in red).

2. **Start Measurements**
    - Start playback and profiling with the *Start Benchmark* button.
    - The results are written to a "benchmark_results" folder in the project root.

3. **Postprocess Results**
    
    After all parameter configurations had been benchmarked, we aggregated the results of all measurements using the Python script `scripts/convert_temporal_measurements.py`. The script expects the following folder structure:
    ```
    measurements
    |-- storage_device_1
        |-- dataset_1
            |-- benchmark_0
            |-- benchmark_5
            |-- benchmark_6
            |-- ...
        |-- dataset_2
        |-- ...
    |-- storage_device_2
    |-- ...
    ```
    For some metrics, the script only writes a CSV with accumulated numbers that we later used to create the charts in the paper (using TikZ/PGFPlots), for others it precomputes TikZ code for performance reasons.

## Reproducing the Experiments for Spatial Exploration
1. **Load point cloud and set parameters**
    - Open a point cloud via the _File_ menu (highlighted in green in the screenshot).
    - Set the correct parameter values in the parameter panel (highlighted in orange)
        - *Projected Size Pixel Threshold*: 250 for Hessigheim, 150 for the others
        - *Benchmarking Mode*: Camera Animation
        - *Benchmark Num Samples*: 100
    - Select the correct *Benchmark Camera Positions* for the dataset . The camera paths used in our evaluation are located in the `camera_paths/spatial_experiments` folder.
    - Add the hole-filling postprocessing step via the + icon in the postprocessing tab (highlighted in red).

2. **Start Measurements**
    - Start playback and profiling with the *Start Benchmark* button.
    - The results are written to a "benchmark_results" folder in the project root.

3. **Postprocess Results**

    After all parameter configurations had been benchmarked, we aggregated the results of all measurements using the Python script `scripts/convert_spatial_measurements.py` (requires matplotlib and numpy). The script expects the same folder structure as the one for the temporal experiments. For some measurements, the script directly creates charts, for others it only writes a CSV with accumulated numbers that we later used for our analysis or to create the charts in the paper.


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

