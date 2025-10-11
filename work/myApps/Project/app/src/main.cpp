#include <stdio.h>
#include <stdlib.h>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h> 
// Removed internal pipeline headers, as they are not needed when using environment variable

// --- Configuration ---
// Frames to acquire before exiting
#define FRAME_COUNT 10
// Listener timeout in milliseconds
#define TIMEOUT_MS 1000

// Helper function to convert the internal 16-bit depth value (mm) to meters (m)
float depth_mm_to_m(uint16_t depth_mm) {
    return (float)depth_mm / 1000.0f;
}

/**
 * @brief Main function to initialize Kinect V2 and capture depth and color data.
 */
int main()
{
    // 1. Initialize freenect2 context
    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev = NULL;
    
    // Updated listener type to include Color, Depth, and IR frames
    int listener_type = libfreenect2::Frame::Depth | libfreenect2::Frame::Ir | libfreenect2::Frame::Color;

    printf("Kinect V2 Depth and Color Capture Example (C++ Language)\n");

    // NOTE: Pipeline selection must now be done via the LIBFREENECT2_PIPELINE environment variable (e.g., 'export LIBFREENECT2_PIPELINE=cpu').

    // 2. Check for connected devices
    if (freenect2.enumerateDevices() == 0)
    {
        fprintf(stderr, "ERROR: No Kinect V2 device connected!\n");
        return EXIT_FAILURE;
    }

    // 3. Open the first connected device
    char serial[256];
    sprintf(serial, "%s", freenect2.getDefaultDeviceSerialNumber().c_str()); 
    printf("Attempting to open device with serial: %s\n", serial);

    // Try to open the device using its serial number
    dev = freenect2.openDevice(serial);

    if (dev == NULL)
    {
        fprintf(stderr, "ERROR: Could not open Kinect V2 device. Check udev rules and USB 3.0 connection.\n");
        return EXIT_FAILURE;
    }

    // 4. Create and set up the Frame Listener
    libfreenect2::SyncMultiFrameListener listener(listener_type);
    
    // Set the listener for all requested frame types
    // CRITICAL FIX: The Freenect2Device API requires setting color and depth/IR listeners separately.
    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);

    // 5. Start the data stream (using the simple no-argument version)
    if (!dev->start()) 
    {
        fprintf(stderr, "ERROR: Could not start device stream.\n");
        dev->close();
        return EXIT_FAILURE;
    }

    // 6. Main capture loop
    libfreenect2::FrameMap frames; 
    int capture_count = 0;

    // Wait for the first frame set to determine resolution dynamically
    if (!listener.waitForNewFrame(frames, 5000)) 
    {
        fprintf(stderr, "WARNING: Timeout occurred while waiting for initial frames. Cannot determine resolution.\n");
    } else {
        libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];
        libfreenect2::Frame *color = frames[libfreenect2::Frame::Color]; // Get color frame for info

        printf("Device stream started successfully.\n");
        printf("Depth frame resolution: %d x %d\n", depth->width, depth->height);
        printf("Color frame resolution: %d x %d\n", color->width, color->height);
        printf("Capturing %d frames and saving depth data to file...\n", FRAME_COUNT);
        listener.release(frames); 
    }
    
    // Now continue with the main capture loop
    while (capture_count < FRAME_COUNT)
    {
        // Wait for a new set of frames
        if (!listener.waitForNewFrame(frames, TIMEOUT_MS)) 
        {
            fprintf(stderr, "WARNING: Timeout occurred while waiting for new frames. Skipping this frame.\n");
            continue;
        }

        // Get the required frames
        libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];
        libfreenect2::Frame *color = frames[libfreenect2::Frame::Color];

        // --- Console Output and Center Depth Calculation ---
        float *depth_data_mm = (float *)depth->data;
        int center_x = depth->width / 2;
        int center_y = depth->height / 2;
        int index = center_y * depth->width + center_x;

        float center_depth_mm = depth_data_mm[index];
        float center_depth_m = center_depth_mm / 1000.0f; // Convert mm to meters

        printf("[%d/%d] Center Depth (Pixel %d, %d): %.2f mm (%.3f m) | Color W: %d, H: %d\n",
               capture_count + 1, FRAME_COUNT, center_x, center_y, center_depth_mm, center_depth_m, color->width, color->height);

        // --- NEW: Save Full Depth Data to File (CSV Format) ---
        char filename[64];
        // Create a unique filename for the depth data (e.g., depth_frame_01.csv)
        snprintf(filename, sizeof(filename), "depth_frame_%02d.csv", capture_count + 1);

        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
            fprintf(stderr, "ERROR: Could not open file %s for writing!\n", filename);
        } else {
            // The depth data is a float array (512*424 elements)
            float *depth_data = (float *)depth->data;
            
            // Write data row by row (424 rows, 512 columns), separating values with a comma
            for (int y = 0; y < depth->height; ++y) {
                for (int x = 0; x < depth->width; ++x) {
                    // Write depth value (mm)
                    fprintf(fp, "%.2f", depth_data[y * depth->width + x]);
                    // Separate values with a comma, unless it's the last column
                    if (x < depth->width - 1) {
                        fprintf(fp, ",");
                    }
                }
                // Newline at the end of the row
                fprintf(fp, "\n");
            }
            printf("Saved depth frame data to %s\n", filename);
            fclose(fp);
        }
        // -------------------------------------

        // Release the frames to allow the listener to capture the next set
        listener.release(frames); 

        capture_count++;
    }

    // 7. Clean up
    dev->stop();
    dev->close();
    printf("Device stopped and closed. Capture complete.\n");

    return EXIT_SUCCESS;
}
