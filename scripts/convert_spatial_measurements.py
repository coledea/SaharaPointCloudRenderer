import numpy as np
import os
import matplotlib.pyplot as plt
import argparse

parser = argparse.ArgumentParser(prog='Script for aggregating the results of the spatial exploration experiments.')
parser.add_argument('input_folder', help='Folder containing the raw measurements. The expected folder hierarchy is storage devices - datasets - configurations.')
parser.add_argument('output_folder', help='Folder to write the aggregated results into.')

timestamp_ranges = [0, 1, 5, 10, 25]
timestamp_falloffs = [0.0, 0.25, 0.5, 0.75, 1.0]
resampling_density = 16000 # 16ms

color_list = [
  '#808080',  # gray
  '#d2e4f0',  # Paired-B!20!white
  '#a5c9e1',  # Paired-B!40!white
  '#79aed2',  # Paired-B!60!white
  '#4c93c3',  # Paired-B!80!white
  '#1f78b4',  # Paired-B

  '#d6ecd5',  # Paired-D!20!white
  '#add9ab',  # Paired-D!40!white
  '#85c680',  # Paired-D!60!white
  '#5cb356',  # Paired-D!80!white
  '#33a02c',  # Paired-D

  '#f9d1d2',  # Paired-F!20!white
  '#f4a3a4',  # Paired-F!40!white
  '#ee7677',  # Paired-F!60!white
  '#e94849',  # Paired-F!80!white
  '#e31a1c',  # Paired-F

  '#e1d8eb',  # Paired-J!20!white
  '#c3b1d7',  # Paired-J!40!white
  '#a68bc2',  # Paired-J!60!white
  '#8864ae',  # Paired-J!80!white
  '#6a3d9a'   # Paired-J
]

args = parser.parse_args()
output_root_folder = args.output_folder

# Loop over all storage devices, scenes, and parameter configurations
for device in os.scandir(args.input_folder):
    if not device.is_dir():
        continue

    output_folder = os.path.join(output_root_folder, device.name)
    os.makedirs(output_folder, exist_ok=True)

    for scene in os.scandir(device.path):
        share_of_cpu_hits = []

        for idx_range, timestamp_range in enumerate(timestamp_ranges):
            for idx_falloff, timestamp_falloff in enumerate(timestamp_falloffs):

                configuration_idx = idx_range * len(timestamp_falloffs) + idx_falloff
                input_folder_path = os.path.join(scene.path, "benchmark_" + str(configuration_idx))
                if not os.path.exists(input_folder_path):
                    continue

                print(device.name, scene.name, configuration_idx)

                # Load raw measurements
                cpu_hits = np.loadtxt(os.path.join(input_folder_path, "CPU_buffer_hits.txt"), delimiter=";", dtype=int)[1:]
                cpu_misses = np.loadtxt(os.path.join(input_folder_path, "CPU_buffer_misses.txt"), delimiter=";", dtype=int)[1:] # skip the first frame
                timestamp_changes = np.loadtxt(os.path.join(input_folder_path, "timestamp_changes.txt"), delimiter=";", dtype=int)
               
               # Resample the measurements using a uniform grid to avoid oversampling for low frame times
                t_uniform = np.arange(cpu_misses[0,0], cpu_misses[-1,0], resampling_density)
                cpu_misses_uniform = np.interp(t_uniform, cpu_misses[:,0], cpu_misses[:,1])
                cpu_hits_uniform = np.interp(t_uniform, cpu_hits[:,0], cpu_hits[:,1])

                # Filter out misses that occured due to timestamp change -> we are only interested in spatial navigation performance
                indices_to_filter = []
                timestamp_switch_indices = np.searchsorted(t_uniform, timestamp_changes[:,0])
                timestamp_switch_indices = np.append(timestamp_switch_indices, len(t_uniform))
                for idx, _ in enumerate(timestamp_changes):
                    index_to_remove = timestamp_switch_indices[idx]
                    while index_to_remove < timestamp_switch_indices[idx+1] and cpu_misses_uniform[index_to_remove] != 0:
                        indices_to_filter.append(index_to_remove)
                        index_to_remove += 1

                cpu_misses_uniform = np.delete(cpu_misses_uniform, indices_to_filter)
                cpu_hits_uniform = np.delete(cpu_hits_uniform, indices_to_filter)
                all_chunks = cpu_hits_uniform + cpu_misses_uniform

                # Filter out all instances, where no chunks were visible (and thus cpu hits and misses are zero), since for these cases the average hit rate is undefined
                no_empty_scene = all_chunks != 0
                all_chunks = all_chunks[no_empty_scene]
                cpu_misses_uniform = cpu_misses_uniform[no_empty_scene]
                cpu_hits_uniform = cpu_hits_uniform[no_empty_scene]

                share_of_cpu_hits.append(cpu_hits_uniform.astype(float) / all_chunks .astype(float))

        # Create violin plots for main memory misses
        fig, ax = plt.subplots()
        fig.patch.set_visible(False)
        ax.margins(x=0.004,y=0.004)
        plt.axis('off')
        ax.get_xaxis().set_visible(False)
        ax.get_yaxis().set_visible(False)
        ax.set_ylim(ymax=1.05, ymin=0)
        ax.set_xlim(xmax=21.5)

        data = [1.0 - entry for entry in share_of_cpu_hits]
        violinplot = ax.violinplot(data, showmedians=False, showextrema=False, showmeans=False, points=500)
        for idx, plot in enumerate(violinplot['bodies']):
            plot.set_facecolor(color_list[idx])
            plot.set_edgecolor('black')
            plot.set_linewidth(0.5)
            plot.set_alpha(1.0)

        # If medians should be marked
        """for idx, entry in enumerate(data):
            ax.plot(idx+1, np.median(entry), marker='x', markersize=2, color='black', markeredgewidth=0.3)"""
        
        fig.set_size_inches(3.5,0.75)
        fig.savefig(os.path.join(output_folder, scene.name + "_cpu_misses_violinplots.pdf"), transparent=None, pad_inches=0.0, bbox_inches=ax.get_window_extent().transformed(fig.dpi_scale_trans.inverted()))

        # Also write out the min, avg, max, and median of misses
        avg_misses = [[np.min(np.array(entry)), np.average(np.array(entry)), np.max(np.array(entry)), np.median(np.array(entry))] for entry in data]
        np.savetxt(os.path.join(output_folder, scene.name +  "_avg_misses.csv"), np.array(avg_misses), fmt="%.5f", delimiter=",", comments="", header="min,avg,max,med")


        # Violinplots for main memory hits -> a bit harder to spot the differences
        """fig, ax = plt.subplots()
        fig.patch.set_visible(False)
        ax.margins(x=0.004,y=0.004)
        plt.axis('off')
        ax.get_xaxis().set_visible(False)
        ax.get_yaxis().set_visible(False)
        ax.set_ylim(ymax=1.1, ymin=0)
        ax.set_xlim(xmax=21.5)

        data = share_of_cpu_hits
        violinplot = ax.violinplot(data, showmedians=False, showextrema=False, showmeans=False, points=400)
        for idx, plot in enumerate(violinplot['bodies']):
            plot.set_facecolor(color_list[idx])
            plot.set_edgecolor('black')
            #plot.set_edgecolor(str(1.0 - (0.5 * (num_outliers[idx] / max_num_outliers))))
            plot.set_linewidth(line_width)
        
        fig.set_size_inches(3.5,0.75)
        fig.savefig(os.path.join(output_folder, scene.name + "_cpu_hits_violinplots.pdf"), transparent=None, pad_inches=0.0, bbox_inches=ax.get_window_extent().transformed(fig.dpi_scale_trans.inverted()))"""