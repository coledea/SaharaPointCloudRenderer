import numpy as np
import os
import csv
from itertools import chain
import warnings
warnings.filterwarnings('error')
import argparse

parser = argparse.ArgumentParser(prog='Script for aggregating the results of the temporal exploration experiments.')
parser.add_argument('input_folder', help='Folder containing the raw measurements. The expected folder hierarchy is storage devices - datasets - configurations.')
parser.add_argument('output_folder', help='Folder to write the aggregated results into.')

scene_display_names = {"hessigheim" : "Hessigheim3D", "liphestream" : "LiPheStream", "schneeferner" : "Schneeferner", "kijkduin" : "Kijkduin", "noordwijk" : "Noordwijk"}

timestamp_ranges = [0, 1, 5, 10, 25]
timestamp_falloffs = [0.0, 0.25, 0.5, 0.75, 1.0]

# For very long measurements, the uint32 timestamps overflowed. We correct this here.
def fix_overflow(measurement):
    overflow = []
    last_t = 0
    for idx, t in enumerate(measurement[:,0]):
        if t < last_t:
            overflow.append(idx)
        last_t = t

    for idx, overflow_start in enumerate(overflow):
        overflow_end = (overflow[idx + 1] if idx + 1 < len(overflow) else None)
        measurement[overflow_start:overflow_end, 0] += np.iinfo(np.uint32).max * (idx + 1)

    return measurement

# Writes value matrix flattened for easier processing in LaTeX
def write_flattened(data, output_path):
    outfile = open(output_path, "w", newline="")
    writer = csv.writer(outfile)
    writer.writerow(["x", "y", "value"])

    for y, row in enumerate(data):
        for x in range(21):
            value = "nan"
            if x < len(row) and not np.isnan(row[x]):
                value = row[x]

            writer.writerow([x, y, value])

    outfile.close()

def group_plot_header(scene_name):
    header = "\\nextgroupplot[\nylabel={\\strut " + scene_display_names[scene_name] + "}"
    if scene_name == "noordwijk":
        header += ",\nxtick style = {draw = none},\nlegend to name=grouplegend"
    header += "\n]\n"
    return header

def get_boxplot_box_pgfstring(data):
    data = np.sort(data)
    median = np.median(data)
    lower_quartile = np.quantile(data, 0.25)
    upper_quartile = np.quantile(data, 0.75)
    iqr = upper_quartile - lower_quartile
    lower_whisker = np.min(np.compress(data >= lower_quartile - 1.5 * iqr, data))
    upper_whisker = np.max(np.compress(data <= upper_quartile + 1.5 * iqr, data))

    result = "\\addplot+ [boxplot prepared = {lower whisker=" + str(lower_whisker) + ", lower quartile=" + str(lower_quartile)
    result += ", median=" + str(median) + ", upper quartile=" + str(upper_quartile) + ", upper whisker=" + str(upper_whisker)
    result += "}] coordinates {};\n\n"
    return result

args = parser.parse_args()
output_root_folder = args.output_folder

for device in os.scandir(args.input_folder):
    if not device.is_dir():
        continue

    frametimes_boxplots = {}
    completion_times_boxplots = {}

    for scene in os.scandir(device.path):
        header = []
        avg_available_node_share_per_distance = []
        avg_frame_completion_per_distance = []
        split_frametime = []

        # group plot header for boxplots
        scene_header = group_plot_header(scene.name)
        frametimes_boxplots[scene.name] = scene_header
        completion_times_boxplots[scene.name] = scene_header

        for idx_range, _ in enumerate(timestamp_ranges):
            for idx_falloff, timestamp_falloff in enumerate(timestamp_falloffs):

                configuration_idx = idx_range * len(timestamp_falloffs) + idx_falloff
                input_folder_path = os.path.join(scene.path, "benchmark_" + str(configuration_idx))
                if not os.path.exists(input_folder_path):
                    continue

                output_folder = os.path.join(output_root_folder, device.name, scene.name)
                os.makedirs(output_folder, exist_ok=True)
                
                header.append("config-" + str(configuration_idx))

                # Load raw measurements
                compute_priorities_time = np.average(np.loadtxt(os.path.join(input_folder_path, "Compute Priorities GPU.txt"), delimiter=";", dtype=np.uint64)[:,1])
                sort_time = np.average(np.loadtxt(os.path.join(input_folder_path, "Update Priorities CPU and Sort.txt"), delimiter=";", dtype=np.uint64)[:,1])
                memory_management_time = np.average(np.loadtxt(os.path.join(input_folder_path, "Update Buffers.txt"), delimiter=";", dtype=np.uint64)[:,1])
                rasterization_time = np.average(np.loadtxt(os.path.join(input_folder_path, "Rasterization.txt"), delimiter=";", dtype=np.uint64)[:,1])
                full_frametime = compute_priorities_time + sort_time + memory_management_time + rasterization_time
                split_frametime.append([float(compute_priorities_time) / float(full_frametime), float(sort_time) / float(full_frametime), float(memory_management_time) / float(full_frametime), float(rasterization_time) / float(full_frametime)])

                frametimes = np.loadtxt(os.path.join(input_folder_path, "Whole_frame.txt"), delimiter=";", dtype=np.uint64)[:,1]
                cpu_misses = np.loadtxt(os.path.join(input_folder_path, "CPU_buffer_misses.txt"), delimiter=";", dtype=np.ulonglong)
                cpu_misses = fix_overflow(cpu_misses)

                cpu_hits = np.loadtxt(os.path.join(input_folder_path, "CPU_buffer_hits.txt"), delimiter=";", dtype=np.uint64)[:,1]
                nodes_to_render = cpu_hits + cpu_misses[:,1]
                
                timestamp_changes = np.loadtxt(os.path.join(input_folder_path, "timestamp_changes.txt"), delimiter=";", dtype=np.ulonglong)
                timestamp_changes = fix_overflow(timestamp_changes)

                # Prepare lists for aggregation
                available_node_share_per_distance = [[] for i in range(timestamp_ranges[-1] + 1)]
                available_node_share_per_distance.append([]) # for switches outside the range

                completion_time_per_distance = [[] for i in range(timestamp_ranges[-1] + 1)]
                completion_time_per_distance.append([]) # for switches outside the range

                # Find points of timestamp switches and determine (1) number of main memory misses directly after switch and (2) time to frame completion
                measurement_start_indices = np.searchsorted(cpu_misses[:,0], timestamp_changes[:,0])
                measurement_start_indices = np.append(measurement_start_indices, len(cpu_misses))
                previous_timestamp = 0 # we always start from timestamp 0
                for idx, timestamp_change in enumerate(timestamp_changes):
                    first_measurement_idx = measurement_start_indices[idx]

                    switch_distance = abs(int(timestamp_change[1]) - previous_timestamp)
                    switch_distance = min(switch_distance, timestamp_ranges[-1] + 1) # switches outside the maximum range are stored under the last index

                    available_node_share_per_distance[switch_distance].append((nodes_to_render[first_measurement_idx] - cpu_misses[first_measurement_idx,1]) / nodes_to_render[first_measurement_idx])

                    # Find frame where all nodes were loaded and compute time difference between start and end of loading
                    index_of_frame_completion = first_measurement_idx
                    while index_of_frame_completion < measurement_start_indices[idx+1] and cpu_misses[index_of_frame_completion, 1] != 0:
                        index_of_frame_completion += 1
                    completion_time_per_distance[switch_distance].append((cpu_misses[index_of_frame_completion,0] - cpu_misses[first_measurement_idx,0]) / 1000)

                    previous_timestamp = int(timestamp_change[1])

                
                avg_available_node_share_per_distance.append([])
                avg_frame_completion_per_distance.append([])
                for dist in range(timestamp_ranges[-1] + 2):
                    # Compute average share of selected nodes available after switch per distance
                    if len(available_node_share_per_distance[dist]) > 0:
                        avg_share_available = np.average(np.array(available_node_share_per_distance[dist]))
                        avg_available_node_share_per_distance[-1].append(avg_share_available)
                    else:
                        avg_available_node_share_per_distance[-1].append(np.nan)

                    # Compute average frame completion time per distance
                    if len(completion_time_per_distance[dist]) > 0:
                        avg_completion_time = np.average(np.array(completion_time_per_distance[dist]))
                        avg_frame_completion_per_distance[-1].append(avg_completion_time)
                    else:
                        avg_frame_completion_per_distance[-1].append(np.nan)

                # Frametime boxplot
                frametimes = frametimes.astype(float) / 1000
                frametimes_boxplots[scene.name] += get_boxplot_box_pgfstring(frametimes)

                # Completion time boxplot
                completion_times = np.array(list(chain.from_iterable(completion_time_per_distance))).astype(float)
                completion_times_boxplots[scene.name] += get_boxplot_box_pgfstring(completion_times)


        # cut invalid rows (no jumps to distance 0, only 3 possible jump distances in Hessigheim)
        valid_rows = timestamp_ranges[-1] + 2
        if scene.name == "hessigheim":
            valid_rows = 4
        avg_available_node_share_per_distance = np.round((np.array(avg_available_node_share_per_distance).T[1:valid_rows] * 100))  # skip first row (no jumps in distance 0)
        avg_frame_completion_per_distance = np.array(avg_frame_completion_per_distance).T[1:valid_rows]
        
        np.savetxt(os.path.join(output_folder, "completion_times_per_distance_absolute.csv"), avg_frame_completion_per_distance, fmt="%d", delimiter=",", comments="", header=",".join(header))

        # since frame completion strongly depends on the specific timestamps jumped to (their size), we normalize by row maximum, i.e., maximum time for a certain switch distance
        frame_completion_max = np.max(avg_frame_completion_per_distance, axis=1, keepdims=True)
        avg_frame_completion_per_distance = (1.0 - (avg_frame_completion_per_distance.astype(float) / frame_completion_max.astype(float))) * 100

        # Write results
        np.savetxt(os.path.join(output_folder, "node_share_available_per_distance.csv"), avg_available_node_share_per_distance, fmt="%.0f", delimiter=",", comments="", header=",".join(header))
        np.savetxt(os.path.join(output_folder, "completion_times_per_distance.csv"), avg_frame_completion_per_distance, fmt="%.2f", delimiter=",", comments="", header=",".join(header))

        write_flattened(avg_available_node_share_per_distance, os.path.join(output_folder, "node_share_available_per_distance_flat.csv"))
        write_flattened(avg_frame_completion_per_distance, os.path.join(output_folder, "completion_times_per_distance_flat.csv"))

        np.savetxt(os.path.join(output_folder, "split_avg_frametimes.csv"), np.array(split_frametime), fmt="%.5f", delimiter=",", comments="", header="priorities,sort,memory,rasterization")


    with open(os.path.join(output_root_folder, device.name, "frametimes_boxplots.txt"), "w") as f:
        for scene in scene_display_names:
            f.write(frametimes_boxplots[scene])
    
    with open(os.path.join(output_root_folder, device.name, "completion_time_boxplots.txt"), "w") as f:
        for scene in scene_display_names:
            f.write(completion_times_boxplots[scene])
