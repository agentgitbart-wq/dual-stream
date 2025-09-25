// Camera Enumeration Utility
// Lists all available camera devices on the system

#include <iostream>
#include <iomanip>
#include "../camera/sources/CameraSourceFactory.h"
#include "../camera/CameraManager.h"
#include "../core/Logger.h"

void PrintDeviceInfo(const CameraDeviceInfo& device, int index) {
    std::cout << "\n[Camera " << index << "]" << std::endl;
    std::cout << "  Name:        " << device.deviceName << std::endl;
    std::cout << "  Type:        " << CameraSourceFactory::GetSourceTypeName(device.type) << std::endl;
    std::cout << "  Index:       " << device.deviceIndex << std::endl;
    
    if (!device.serialNumber.empty()) {
        std::cout << "  Serial:      " << device.serialNumber << std::endl;
    }
    
    std::cout << "  Capabilities:" << std::endl;
    std::cout << "    - Max Resolution: " << device.maxWidth << "x" << device.maxHeight << std::endl;
    std::cout << "    - Max FPS:        " << device.maxFrameRate << std::endl;
    std::cout << "    - Depth Support:  " << (device.supportsDepth ? "Yes" : "No") << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "Camera Enumeration Utility" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Check if specific camera type was requested
    bool showOpenCV = true;
    bool showRealSense = true;
    
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--opencv") {
            showRealSense = false;
        } else if (arg == "--realsense") {
            showOpenCV = false;
        } else if (arg == "--help") {
            std::cout << "\nUsage: list_cameras [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --opencv     Show only OpenCV cameras" << std::endl;
            std::cout << "  --realsense  Show only RealSense cameras" << std::endl;
            std::cout << "  --help       Show this help message" << std::endl;
            return 0;
        }
    }
    
    // Enumerate all devices
    std::cout << "\nEnumerating all camera devices..." << std::endl;
    auto allDevices = CameraManager::EnumerateDevices();
    
    if (allDevices.empty()) {
        std::cout << "\nNo camera devices found!" << std::endl;
        return 1;
    }
    
    std::cout << "\nFound " << allDevices.size() << " camera device(s):" << std::endl;
    
    // Display OpenCV cameras
    if (showOpenCV) {
        std::cout << "\n--- OpenCV Cameras (USB/Built-in) ---" << std::endl;
        auto opencvDevices = CameraManager::EnumerateDevices(CameraSourceType::OPENCV_WEBCAM);
        
        if (opencvDevices.empty()) {
            std::cout << "  No OpenCV cameras found" << std::endl;
        } else {
            int index = 0;
            for (const auto& device : opencvDevices) {
                PrintDeviceInfo(device, index++);
            }
        }
    }
    
    // Display RealSense cameras
    if (showRealSense) {
        std::cout << "\n--- Intel RealSense Cameras ---" << std::endl;
        auto realsenseDevices = CameraManager::EnumerateDevices(CameraSourceType::REALSENSE_DEVICE);
        
        if (realsenseDevices.empty()) {
            std::cout << "  No RealSense cameras found" << std::endl;
        } else {
            int index = 0;
            for (const auto& device : realsenseDevices) {
                PrintDeviceInfo(device, index++);
            }
        }
    }
    
    // Show usage instructions
    std::cout << "\n========================================" << std::endl;
    std::cout << "To use a specific camera with dual_stream:" << std::endl;
    std::cout << "\nFor OpenCV camera (by index):" << std::endl;
    std::cout << "  dual_stream.exe video1.mp4 video2.mp4 --trigger face --camera-index 0" << std::endl;
    std::cout << "\nFor RealSense camera (by serial or first available):" << std::endl;
    std::cout << "  dual_stream.exe video1.mp4 video2.mp4 --trigger face --camera-type realsense" << std::endl;
    std::cout << "\nFor specific RealSense (by serial number):" << std::endl;
    std::cout << "  dual_stream.exe video1.mp4 video2.mp4 --trigger face --camera-serial 123456789" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}