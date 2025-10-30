import uproot
import numpy as np
import random
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

# Open the ROOT file
file_path = "digi_Run00001.root"
with uproot.open(file_path) as file:
    # Access the eventInfo TTree
    tree = file["event_info"]
    
    # Read the redpix_ix, redpix_iy, redpix_iz, and nRedpix branches
    redpix_ix = tree["redpix_ix"].array(library="np")
    redpix_iy = tree["redpix_iy"].array(library="np")
    redpix_iz = tree["redpix_iz"].array(library="np")
    nRedpix = tree["nRedpix"].array(library="np")

# Isolate tracks
tracks = []
for event_idx in range(len(nRedpix)):
    start_idx = 0  # Tracks start at index 0
    track_length = nRedpix[event_idx]
    
    # Extract the track for the current event
    track_ix = redpix_ix[event_idx][start_idx: start_idx + track_length]
    track_iy = redpix_iy[event_idx][start_idx: start_idx + track_length]
    track_iz = redpix_iz[event_idx][start_idx: start_idx + track_length]
    
    # Store the track as a dictionary
    tracks.append({
        "ix": track_ix,
        "iy": track_iy,
        "iz": track_iz
    })
"""
# Print the isolated tracks
for i, track in enumerate(tracks):
    print(f"Track {i}:")
    print(f"  ix: {track['ix']}")
    print(f"  iy: {track['iy']}")
    print(f"  iz: {track['iz']}")
    
"""
#! Plot 25 random 3D histograms containing one track each

# Select 16 random tracks
random_tracks = random.sample(tracks, min(16, len(tracks)))

# Create a figure with 16 subplots
fig, axes = plt.subplots(4, 4, figsize=(20, 20))

# Flatten the axes array for easier indexing
axes = axes.flatten()

# Loop through the selected random tracks and plot 2D histograms
for idx, track in enumerate(random_tracks):
    ax = axes[idx]
    
    # Calculate the range for bins as all integers in the range of ix and iy
    ix_bins = np.arange(np.min(track["ix"]), np.max(track["ix"]) + 2)  # +2 to include the last bin
    iy_bins = np.arange(np.min(track["iy"]), np.max(track["iy"]) + 2)  # +2 to include the last bin
    
    h = ax.hist2d(
        track["ix"], track["iy"], 
        bins=[ix_bins, iy_bins], 
        weights=track["iz"], 
        cmap="viridis"
    )
    ax.set_title(f"Track {idx}")
    ax.set_xlabel("ix")
    ax.set_ylabel("iy")
    ax.set_xlim(ix_bins[0], ix_bins[-1])
    ax.set_ylim(iy_bins[0], iy_bins[-1])

# Adjust layout to make room for the colorbar
fig.tight_layout(rect=[0, 0, 0.9, 1])  # Leave space on the right for the colorbar

# Add a colorbar
cbar = fig.colorbar(h[3], ax=axes, orientation='vertical', fraction=0.02, pad=0.02)
cbar.set_label("redpix_iz")

# Show the plot
plt.show()

#! Create a single 2D histogram summing up all tracks
fig, ax = plt.subplots(figsize=(24, 12))  # Adjust the figure size for a rectangular plot

# Define the range for the bins
ix_bins = np.arange(0, 4096)  # 0 to 4095
iy_bins = np.arange(0, 2304)  # 0 to 2303

# Initialize arrays to accumulate the data
accumulated_ix = []
accumulated_iy = []
accumulated_iz = []

# Accumulate data from all tracks
for track in tracks:
    accumulated_ix.extend(track["ix"])
    accumulated_iy.extend(track["iy"])
    accumulated_iz.extend(track["iz"])

# Create the 2D histogram
h = ax.hist2d(
    accumulated_ix, accumulated_iy, 
    bins=[ix_bins, iy_bins], 
    weights=accumulated_iz, 
    cmap="viridis"
)

# Add labels and title
ax.set_title("Summed 2D Histogram of All Tracks")
ax.set_xlabel("ix")
ax.set_ylabel("iy")
ax.set_xlim(0, 4095)
ax.set_ylim(0, 2303)

# Adjust the aspect ratio to reflect the bin ranges
#ax.set_aspect((2304 / 4096) )  # Adjust for the correct rectangular aspect ratio

# Add a colorbar
cbar = fig.colorbar(h[3], ax=ax, orientation='vertical')
cbar.set_label("Summed redpix_iz")

# Show the plot
plt.show()

