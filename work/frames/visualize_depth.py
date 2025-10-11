import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys
import os
import glob

def load_frames(pattern):
    """
    Finds all CSV files matching the pattern, loads them, and sorts them.
    Returns a list of NumPy arrays (frames) and the sorted file paths.
    """
    # Use glob to find all files matching the pattern
    file_paths = sorted(glob.glob(pattern))

    if not file_paths:
        print(f"Error: No files found matching pattern '{pattern}'")
        return None, None

    frames_data = []
    
    print(f"Found {len(file_paths)} files. Loading...")
    
    for filepath in file_paths:
        try:
            # Load the data. depth_viewer.c saves the data as floats separated by commas.
            depth_data = np.loadtxt(filepath, delimiter=',', dtype=np.float32)

            if depth_data.ndim == 2:
                # The data contains '0' for invalid/unmeasured depth. Mask these out.
                depth_data_masked = np.where(depth_data > 0, depth_data, np.nan)
                frames_data.append(depth_data_masked)
        except Exception as e:
            print(f"Warning: Could not load {filepath}. Error: {e}")
            continue

    if not frames_data:
        print("Error: No valid depth frames were loaded.")
        return None, None
        
    return frames_data, file_paths

def animate_depth_frames(frames_data, file_paths, interval_ms=200):
    """
    Animates the loaded depth frames as a time sequence.
    """
    fig, ax = plt.subplots(figsize=(10, 8))
    
    # Get initial frame and its shape
    initial_frame = frames_data[0]
    num_frames = len(frames_data)
    
    # Determine min/max depth for consistent color mapping across all frames
    # Flatten all frames into a single array, ignore NaNs
    all_data = np.concatenate([f[~np.isnan(f)] for f in frames_data if not np.all(np.isnan(f))])
    vmax = np.nanmax(all_data) if all_data.size > 0 else 5000 # Max 5000mm or the max data value
    vmin = np.nanmin(all_data) if all_data.size > 0 else 500 # Min 500mm or the min data value

    # Initialize the image plot using the first frame
    img = ax.imshow(
        initial_frame, 
        cmap='viridis_r', 
        interpolation='none',
        vmin=vmin,  # Set fixed minimum value
        vmax=vmax   # Set fixed maximum value
    )
    
    # Add color bar
    cbar = fig.colorbar(img, ax=ax, label='Depth (mm)')
    
    ax.set_title(f"Kinect V2 Depth Map Animation (Frame 1/{num_frames})")
    ax.set_xlabel("X Pixel Coordinate (512)")
    ax.set_ylabel("Y Pixel Coordinate (424)")
    # The ax.invert_yaxis() command was removed here to fix the upside-down issue.

    # The function that updates the frame data for the animation
    def update(frame_index):
        frame = frames_data[frame_index]
        # Set the new data for the image plot
        img.set_array(frame)
        
        # Update the title to show the current frame number and file name
        ax.set_title(f"Kinect V2 Depth Map Animation (Frame {frame_index + 1}/{num_frames})\nFile: {os.path.basename(file_paths[frame_index])}")
        
        # We must return the iterable of artists that were modified
        return [img]

    # Create the animation object
    ani = animation.FuncAnimation(
        fig, 
        update, 
        frames=num_frames, 
        interval=interval_ms, # Delay between frames in milliseconds (5 FPS default)
        blit=True, # Optimized drawing only of changed parts
        repeat=True # Loop the animation
    )

    # Show the plot and start the animation
    print("\nAnimation started. Press Ctrl+C in the terminal to stop the script.")
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 visualize_depth.py <path_to_depth_csv_pattern>")
        print("Example: python3 visualize_depth.py 'depth_frame_*.csv'")
        print("NOTE: Remember to quote the pattern ('...') on the command line.")
    else:
        # The first argument is now treated as a file pattern
        pattern = sys.argv[1]
        
        # Load all frames
        frames, file_paths = load_frames(pattern)

        if frames is not None:
            # Start the animation with a 200ms interval (5 FPS)
            animate_depth_frames(frames, file_paths, interval_ms=200)

