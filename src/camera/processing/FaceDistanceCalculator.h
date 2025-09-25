#pragma once

#include "../CameraFrame.h"
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @file FaceDistanceCalculator.h
 * @brief Face distance calculation using RealSense depth data
 * 
 * This utility provides distance calculation for detected faces using Intel RealSense
 * depth cameras. It integrates with the existing face detection system to provide
 * precise distance measurements for the original streamer's switching logic.
 */

namespace DualStream {

/**
 * @brief Face information with distance data
 * 
 * Extended face information that includes distance calculation results
 * for integration with the original streamer's sophisticated switching logic.
 */
struct FaceDistanceInfo {
    cv::Rect rect;              ///< Face bounding rectangle
    float confidence = 0.0f;    ///< Detection confidence score
    double distance = -1.0;     ///< Distance to face in millimeters (-1 = invalid)
    bool hasValidDistance = false;  ///< Whether distance calculation was successful
    
    // Additional data for enhanced processing
    cv::Point2f centerPoint;    ///< Center point of face
    double averageDepth = -1.0; ///< Average depth value in face region
    double medianDepth = -1.0;  ///< Median depth value (more robust to outliers)
    int validDepthPixels = 0;   ///< Number of valid depth pixels in face region
    
    FaceDistanceInfo() = default;
    FaceDistanceInfo(const cv::Rect& faceRect, float conf = 0.0f) 
        : rect(faceRect), confidence(conf) {
        centerPoint = cv::Point2f(faceRect.x + faceRect.width / 2.0f, 
                                 faceRect.y + faceRect.height / 2.0f);
    }
};

/**
 * @brief Configuration for distance calculation
 */
struct DistanceCalculationConfig {
    // Distance calculation parameters
    double minValidDistance = 100.0;    ///< Minimum valid distance in mm
    double maxValidDistance = 5000.0;   ///< Maximum valid distance in mm
    
    // Depth processing parameters
    double maxDepthVariance = 200.0;    ///< Maximum depth variance for valid region
    int minValidPixels = 10;            ///< Minimum valid depth pixels required
    bool useMedianDepth = true;         ///< Use median instead of average (more robust)
    bool filterOutliers = true;         ///< Filter depth outliers before calculation
    
    // RealSense specific parameters
    double depthScale = 1.0;            ///< Depth scale factor (usually from RealSense)
    bool enableTemporalFiltering = true; ///< Enable temporal filtering for stability
    
    // Sampling parameters for performance
    int samplingStep = 2;               ///< Pixel sampling step (1 = every pixel, 2 = every 2nd pixel)
    bool useCenterRegion = true;        ///< Focus on center region of face
    double centerRegionRatio = 0.6;     ///< Ratio of face region to use for center sampling
};

/**
 * @brief Utility class for calculating face distances using RealSense depth data
 * 
 * This class provides comprehensive distance calculation functionality for face detection
 * results, integrating with Intel RealSense depth cameras to provide accurate distance
 * measurements for the original streamer's sophisticated switching logic.
 */
class FaceDistanceCalculator {
public:
    explicit FaceDistanceCalculator(const DistanceCalculationConfig& config = DistanceCalculationConfig());
    ~FaceDistanceCalculator() = default;
    
    // Core functionality
    
    /**
     * @brief Calculate distances for detected faces
     * @param faces Vector of detected face rectangles
     * @param frame Camera frame with depth data
     * @return Vector of face distance information
     */
    std::vector<FaceDistanceInfo> CalculateDistances(const std::vector<cv::Rect>& faces, 
                                                    const CameraFrame& frame);
    
    /**
     * @brief Calculate distance for a single face
     * @param faceRect Face bounding rectangle
     * @param frame Camera frame with depth data
     * @return Face distance information
     */
    FaceDistanceInfo CalculateDistance(const cv::Rect& faceRect, const CameraFrame& frame);
    
    /**
     * @brief Extract depth value at specific point
     * @param point Point in image coordinates
     * @param frame Camera frame with depth data
     * @return Depth value in millimeters (-1 if invalid)
     */
    double GetDepthAtPoint(const cv::Point2f& point, const CameraFrame& frame);
    
    /**
     * @brief Get depth values for a region
     * @param region Rectangle region in image coordinates
     * @param frame Camera frame with depth data
     * @return Vector of valid depth values in the region
     */
    std::vector<double> GetDepthInRegion(const cv::Rect& region, const CameraFrame& frame);
    
    // Configuration management
    
    /**
     * @brief Update calculation configuration
     * @param config New configuration
     */
    void UpdateConfig(const DistanceCalculationConfig& config);
    
    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    const DistanceCalculationConfig& GetConfig() const { return m_config; }
    
    // Utility functions for integration with original streamer logic
    
    /**
     * @brief Convert face distance information to original streamer format
     * @param faceDistances Vector of face distance information
     * @return Vector of distances compatible with original streamer's EYELF structure
     */
    static std::vector<double> ExtractDistanceVector(const std::vector<FaceDistanceInfo>& faceDistances);
    
    /**
     * @brief Filter faces by distance criteria (for switching logic)
     * @param faceDistances Vector of face distance information
     * @param minDistance Minimum distance filter
     * @param maxDistance Maximum distance filter
     * @return Filtered vector of face distance information
     */
    static std::vector<FaceDistanceInfo> FilterByDistance(const std::vector<FaceDistanceInfo>& faceDistances,
                                                         double minDistance, double maxDistance);
    
    /**
     * @brief Get closest face from the vector
     * @param faceDistances Vector of face distance information
     * @return Closest valid face (empty FaceDistanceInfo if none found)
     */
    static FaceDistanceInfo GetClosestFace(const std::vector<FaceDistanceInfo>& faceDistances);
    
    /**
     * @brief Check if frame has valid depth data for distance calculation
     * @param frame Camera frame to check
     * @return True if frame has usable depth data
     */
    static bool HasValidDepthData(const CameraFrame& frame);
    
    // Statistics and debugging
    
    /**
     * @brief Get calculation statistics
     * @return String with performance and accuracy statistics
     */
    std::string GetStatistics() const;
    
    /**
     * @brief Reset calculation statistics
     */
    void ResetStatistics();

private:
    DistanceCalculationConfig m_config;
    
    // Statistics tracking
    mutable size_t m_totalCalculations = 0;
    mutable size_t m_successfulCalculations = 0;
    mutable size_t m_failedCalculations = 0;
    mutable double m_averageCalculationTime = 0.0;
    
    // Temporal filtering for stability
    std::vector<double> m_previousDistances;
    std::chrono::steady_clock::time_point m_lastCalculationTime;
    
    // Private calculation methods
    
    /**
     * @brief Calculate raw distance for face region
     * @param faceRect Face bounding rectangle
     * @param depthData Pointer to depth data
     * @param depthWidth Depth image width
     * @param depthHeight Depth image height
     * @param frameWidth Frame width for coordinate mapping
     * @param frameHeight Frame height for coordinate mapping
     * @return Raw distance information
     */
    FaceDistanceInfo CalculateRawDistance(const cv::Rect& faceRect, 
                                        const uint8_t* depthData,
                                        int depthWidth, int depthHeight,
                                        int frameWidth, int frameHeight);
    
    /**
     * @brief Apply temporal filtering to distance measurement
     * @param newDistance New distance measurement
     * @param faceIndex Index of face for tracking
     * @return Filtered distance
     */
    double ApplyTemporalFilter(double newDistance, size_t faceIndex);
    
    /**
     * @brief Extract depth value from depth data
     * @param x X coordinate in depth image
     * @param y Y coordinate in depth image
     * @param depthData Pointer to depth data
     * @param depthWidth Depth image width
     * @param depthHeight Depth image height
     * @return Depth value in millimeters
     */
    double ExtractDepthValue(int x, int y, const uint8_t* depthData, 
                           int depthWidth, int depthHeight) const;
    
    /**
     * @brief Calculate statistics for depth values
     * @param depthValues Vector of depth values
     * @param outAverage Output average depth
     * @param outMedian Output median depth
     * @param outValidCount Output count of valid pixels
     */
    void CalculateDepthStatistics(const std::vector<double>& depthValues,
                                double& outAverage, double& outMedian, int& outValidCount) const;
    
    /**
     * @brief Filter outlier depth values
     * @param depthValues Vector of depth values to filter
     * @return Filtered vector of depth values
     */
    std::vector<double> FilterDepthOutliers(const std::vector<double>& depthValues) const;
    
    /**
     * @brief Map color image coordinates to depth image coordinates
     * @param colorX X coordinate in color image
     * @param colorY Y coordinate in color image
     * @param colorWidth Color image width
     * @param colorHeight Color image height
     * @param depthWidth Depth image width
     * @param depthHeight Depth image height
     * @param outDepthX Output X coordinate in depth image
     * @param outDepthY Output Y coordinate in depth image
     */
    void MapColorToDepthCoordinates(int colorX, int colorY, 
                                  int colorWidth, int colorHeight,
                                  int depthWidth, int depthHeight,
                                  int& outDepthX, int& outDepthY) const;
    
    /**
     * @brief Update calculation statistics
     * @param calculationTime Time taken for calculation
     * @param wasSuccessful Whether calculation was successful
     */
    void UpdateStatistics(double calculationTime, bool wasSuccessful) const;
};

// Convenience functions for integration with existing code

/**
 * @brief Calculate face distances from simple face rectangles
 * @param faces Vector of face rectangles
 * @param frame Camera frame with depth data
 * @param config Distance calculation configuration
 * @return Vector of distances in millimeters
 */
std::vector<double> CalculateFaceDistances(const std::vector<cv::Rect>& faces, 
                                         const CameraFrame& frame,
                                         const DistanceCalculationConfig& config = DistanceCalculationConfig());

/**
 * @brief Check if any face is within distance range
 * @param faceDistances Vector of face distances
 * @param minDistance Minimum distance threshold
 * @param maxDistance Maximum distance threshold
 * @return True if any face is within the specified range
 */
bool HasFaceInRange(const std::vector<double>& faceDistances, double minDistance, double maxDistance);

/**
 * @brief Count faces within distance range
 * @param faceDistances Vector of face distances
 * @param minDistance Minimum distance threshold
 * @param maxDistance Maximum distance threshold
 * @return Number of faces within the specified range
 */
int CountFacesInRange(const std::vector<double>& faceDistances, double minDistance, double maxDistance);

} // namespace DualStream