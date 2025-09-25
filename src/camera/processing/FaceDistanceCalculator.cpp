#include "FaceDistanceCalculator.h"
#include "../../core/Logger.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <chrono>

namespace DualStream {

// FaceDistanceCalculator Implementation

FaceDistanceCalculator::FaceDistanceCalculator(const DistanceCalculationConfig& config)
    : m_config(config) {
    m_lastCalculationTime = std::chrono::steady_clock::now();
}

std::vector<FaceDistanceInfo> FaceDistanceCalculator::CalculateDistances(const std::vector<cv::Rect>& faces, 
                                                                       const CameraFrame& frame) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::vector<FaceDistanceInfo> results;
    results.reserve(faces.size());
    
    if (!HasValidDepthData(frame)) {
        LOG_WARNING("Frame does not contain valid depth data for distance calculation");
        // Return faces with invalid distances
        for (const auto& face : faces) {
            results.emplace_back(face, 0.0f);
        }
        return results;
    }
    
    // Calculate distance for each face
    for (const auto& face : faces) {
        FaceDistanceInfo distanceInfo = CalculateDistance(face, frame);
        results.push_back(distanceInfo);
    }
    
    // Apply temporal filtering if enabled
    if (m_config.enableTemporalFiltering) {
        for (size_t i = 0; i < results.size(); ++i) {
            if (results[i].hasValidDistance) {
                results[i].distance = ApplyTemporalFilter(results[i].distance, i);
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    bool allSuccessful = std::all_of(results.begin(), results.end(), 
                                   [](const FaceDistanceInfo& info) { return info.hasValidDistance; });
    UpdateStatistics(duration, allSuccessful);
    
    LOG_TRACE("Calculated distances for {} faces in {:.2f}ms", faces.size(), duration);
    
    return results;
}

FaceDistanceInfo FaceDistanceCalculator::CalculateDistance(const cv::Rect& faceRect, const CameraFrame& frame) {
    FaceDistanceInfo result(faceRect, 0.0f);
    
    if (!HasValidDepthData(frame)) {
        return result;
    }
    
    // Map face rectangle coordinates to depth image coordinates
    int depthWidth = frame.realsense.depthWidth;
    int depthHeight = frame.realsense.depthHeight;
    
    if (depthWidth <= 0 || depthHeight <= 0 || frame.realsense.depthData == nullptr) {
        LOG_WARNING("Invalid depth data dimensions: {}x{}", depthWidth, depthHeight);
        return result;
    }
    
    // Calculate raw distance using depth data
    result = CalculateRawDistance(faceRect, frame.realsense.depthData, 
                                depthWidth, depthHeight, frame.width, frame.height);
    
    // Apply filtering and validation
    if (result.hasValidDistance) {
        if (result.distance < m_config.minValidDistance || result.distance > m_config.maxValidDistance) {
            LOG_TRACE("Distance {:.1f}mm outside valid range [{:.1f}, {:.1f}]mm", 
                     result.distance, m_config.minValidDistance, m_config.maxValidDistance);
            result.hasValidDistance = false;
            result.distance = -1.0;
        }
    }
    
    return result;
}

double FaceDistanceCalculator::GetDepthAtPoint(const cv::Point2f& point, const CameraFrame& frame) {
    if (!HasValidDepthData(frame)) {
        return -1.0;
    }
    
    int depthX, depthY;
    MapColorToDepthCoordinates(static_cast<int>(point.x), static_cast<int>(point.y),
                              frame.width, frame.height,
                              frame.realsense.depthWidth, frame.realsense.depthHeight,
                              depthX, depthY);
    
    return ExtractDepthValue(depthX, depthY, frame.realsense.depthData, 
                           frame.realsense.depthWidth, frame.realsense.depthHeight);
}

std::vector<double> FaceDistanceCalculator::GetDepthInRegion(const cv::Rect& region, const CameraFrame& frame) {
    std::vector<double> depthValues;
    
    if (!HasValidDepthData(frame)) {
        return depthValues;
    }
    
    // Sample the region with configurable step size for performance
    for (int y = region.y; y < region.y + region.height; y += m_config.samplingStep) {
        for (int x = region.x; x < region.x + region.width; x += m_config.samplingStep) {
            double depth = GetDepthAtPoint(cv::Point2f(x, y), frame);
            if (depth > 0) {
                depthValues.push_back(depth);
            }
        }
    }
    
    // Filter outliers if enabled
    if (m_config.filterOutliers && !depthValues.empty()) {
        depthValues = FilterDepthOutliers(depthValues);
    }
    
    return depthValues;
}

void FaceDistanceCalculator::UpdateConfig(const DistanceCalculationConfig& config) {
    m_config = config;
    LOG_DEBUG("Updated face distance calculation configuration");
}

// Static utility functions

std::vector<double> FaceDistanceCalculator::ExtractDistanceVector(const std::vector<FaceDistanceInfo>& faceDistances) {
    std::vector<double> distances;
    distances.reserve(faceDistances.size());
    
    for (const auto& faceInfo : faceDistances) {
        if (faceInfo.hasValidDistance) {
            distances.push_back(faceInfo.distance);
        }
    }
    
    return distances;
}

std::vector<FaceDistanceInfo> FaceDistanceCalculator::FilterByDistance(const std::vector<FaceDistanceInfo>& faceDistances,
                                                                      double minDistance, double maxDistance) {
    std::vector<FaceDistanceInfo> filtered;
    
    for (const auto& faceInfo : faceDistances) {
        if (faceInfo.hasValidDistance && 
            faceInfo.distance >= minDistance && 
            faceInfo.distance <= maxDistance) {
            filtered.push_back(faceInfo);
        }
    }
    
    return filtered;
}

FaceDistanceInfo FaceDistanceCalculator::GetClosestFace(const std::vector<FaceDistanceInfo>& faceDistances) {
    FaceDistanceInfo closest;
    double closestDistance = std::numeric_limits<double>::max();
    
    for (const auto& faceInfo : faceDistances) {
        if (faceInfo.hasValidDistance && faceInfo.distance < closestDistance) {
            closest = faceInfo;
            closestDistance = faceInfo.distance;
        }
    }
    
    return closest;
}

bool FaceDistanceCalculator::HasValidDepthData(const CameraFrame& frame) {
    return frame.type == CameraFrameType::REALSENSE_FRAME &&
           frame.realsense.depthData != nullptr &&
           frame.realsense.depthWidth > 0 &&
           frame.realsense.depthHeight > 0;
}

std::string FaceDistanceCalculator::GetStatistics() const {
    if (m_totalCalculations == 0) {
        return "No calculations performed yet";
    }
    
    double successRate = static_cast<double>(m_successfulCalculations) / m_totalCalculations * 100.0;
    
    return "Distance Calculation Statistics:\n"
           "- Total calculations: " + std::to_string(m_totalCalculations) + "\n"
           "- Successful: " + std::to_string(m_successfulCalculations) + "\n"
           "- Failed: " + std::to_string(m_failedCalculations) + "\n"
           "- Success rate: " + std::to_string(successRate) + "%\n"
           "- Average time: " + std::to_string(m_averageCalculationTime) + "ms";
}

void FaceDistanceCalculator::ResetStatistics() {
    m_totalCalculations = 0;
    m_successfulCalculations = 0;
    m_failedCalculations = 0;
    m_averageCalculationTime = 0.0;
}

// Private method implementations

FaceDistanceInfo FaceDistanceCalculator::CalculateRawDistance(const cv::Rect& faceRect, 
                                                            const uint8_t* depthData,
                                                            int depthWidth, int depthHeight,
                                                            int frameWidth, int frameHeight) {
    FaceDistanceInfo result(faceRect, 0.0f);
    
    // Determine sampling region (center region or full face)
    cv::Rect samplingRect = faceRect;
    if (m_config.useCenterRegion) {
        int centerWidth = static_cast<int>(faceRect.width * m_config.centerRegionRatio);
        int centerHeight = static_cast<int>(faceRect.height * m_config.centerRegionRatio);
        int centerX = faceRect.x + (faceRect.width - centerWidth) / 2;
        int centerY = faceRect.y + (faceRect.height - centerHeight) / 2;
        samplingRect = cv::Rect(centerX, centerY, centerWidth, centerHeight);
    }
    
    // Collect depth values from the sampling region
    std::vector<double> depthValues;
    
    for (int y = samplingRect.y; y < samplingRect.y + samplingRect.height; y += m_config.samplingStep) {
        for (int x = samplingRect.x; x < samplingRect.x + samplingRect.width; x += m_config.samplingStep) {
            // Map color coordinates to depth coordinates
            int depthX, depthY;
            MapColorToDepthCoordinates(x, y, frameWidth, frameHeight, depthWidth, depthHeight, depthX, depthY);
            
            // Extract depth value
            double depth = ExtractDepthValue(depthX, depthY, depthData, depthWidth, depthHeight);
            if (depth > 0) {
                depthValues.push_back(depth);
            }
        }
    }
    
    // Check if we have enough valid depth pixels
    if (depthValues.size() < static_cast<size_t>(m_config.minValidPixels)) {
        LOG_TRACE("Insufficient valid depth pixels: {} < {}", depthValues.size(), m_config.minValidPixels);
        return result;
    }
    
    // Filter outliers if enabled
    if (m_config.filterOutliers) {
        depthValues = FilterDepthOutliers(depthValues);
        
        if (depthValues.size() < static_cast<size_t>(m_config.minValidPixels)) {
            LOG_TRACE("Insufficient valid depth pixels after outlier filtering: {}", depthValues.size());
            return result;
        }
    }
    
    // Calculate statistics
    double averageDepth, medianDepth;
    int validCount;
    CalculateDepthStatistics(depthValues, averageDepth, medianDepth, validCount);
    
    // Choose distance calculation method
    double finalDistance = m_config.useMedianDepth ? medianDepth : averageDepth;
    
    // Apply depth scale
    finalDistance *= m_config.depthScale;
    
    // Fill result structure
    result.distance = finalDistance;
    result.averageDepth = averageDepth * m_config.depthScale;
    result.medianDepth = medianDepth * m_config.depthScale;
    result.validDepthPixels = validCount;
    result.hasValidDistance = true;
    
    LOG_TRACE("Face distance calculated: {:.1f}mm (avg: {:.1f}, median: {:.1f}, pixels: {})", 
             result.distance, result.averageDepth, result.medianDepth, result.validDepthPixels);
    
    return result;
}

double FaceDistanceCalculator::ApplyTemporalFilter(double newDistance, size_t faceIndex) {
    // Expand previous distances vector if needed
    while (m_previousDistances.size() <= faceIndex) {
        m_previousDistances.push_back(-1.0);
    }
    
    // Simple temporal filtering (can be enhanced with more sophisticated algorithms)
    if (m_previousDistances[faceIndex] > 0) {
        const double alpha = 0.3;  // Smoothing factor
        double filteredDistance = alpha * newDistance + (1.0 - alpha) * m_previousDistances[faceIndex];
        m_previousDistances[faceIndex] = filteredDistance;
        return filteredDistance;
    } else {
        m_previousDistances[faceIndex] = newDistance;
        return newDistance;
    }
}

double FaceDistanceCalculator::ExtractDepthValue(int x, int y, const uint8_t* depthData, 
                                               int depthWidth, int depthHeight) const {
    if (x < 0 || x >= depthWidth || y < 0 || y >= depthHeight || depthData == nullptr) {
        return -1.0;
    }
    
    // RealSense depth data is typically 16-bit (2 bytes per pixel)
    int index = (y * depthWidth + x) * 2;
    uint16_t depthValue = *reinterpret_cast<const uint16_t*>(&depthData[index]);
    
    return static_cast<double>(depthValue);
}

void FaceDistanceCalculator::CalculateDepthStatistics(const std::vector<double>& depthValues,
                                                    double& outAverage, double& outMedian, int& outValidCount) const {
    if (depthValues.empty()) {
        outAverage = outMedian = -1.0;
        outValidCount = 0;
        return;
    }
    
    outValidCount = static_cast<int>(depthValues.size());
    
    // Calculate average
    outAverage = std::accumulate(depthValues.begin(), depthValues.end(), 0.0) / depthValues.size();
    
    // Calculate median
    std::vector<double> sortedValues = depthValues;
    std::sort(sortedValues.begin(), sortedValues.end());
    
    size_t middle = sortedValues.size() / 2;
    if (sortedValues.size() % 2 == 0) {
        outMedian = (sortedValues[middle - 1] + sortedValues[middle]) / 2.0;
    } else {
        outMedian = sortedValues[middle];
    }
}

std::vector<double> FaceDistanceCalculator::FilterDepthOutliers(const std::vector<double>& depthValues) const {
    if (depthValues.size() < 3) {
        return depthValues;  // Not enough data for outlier detection
    }
    
    // Calculate mean and standard deviation
    double mean = std::accumulate(depthValues.begin(), depthValues.end(), 0.0) / depthValues.size();
    
    double variance = 0.0;
    for (double value : depthValues) {
        variance += (value - mean) * (value - mean);
    }
    variance /= depthValues.size();
    double stddev = std::sqrt(variance);
    
    // Filter values outside 2 standard deviations
    std::vector<double> filtered;
    for (double value : depthValues) {
        if (std::abs(value - mean) <= 2.0 * stddev) {
            filtered.push_back(value);
        }
    }
    
    return filtered.empty() ? depthValues : filtered;
}

void FaceDistanceCalculator::MapColorToDepthCoordinates(int colorX, int colorY, 
                                                      int colorWidth, int colorHeight,
                                                      int depthWidth, int depthHeight,
                                                      int& outDepthX, int& outDepthY) const {
    // Simple proportional mapping (can be enhanced with proper camera calibration)
    outDepthX = static_cast<int>((static_cast<double>(colorX) / colorWidth) * depthWidth);
    outDepthY = static_cast<int>((static_cast<double>(colorY) / colorHeight) * depthHeight);
    
    // Clamp to depth image bounds
    outDepthX = std::max(0, std::min(outDepthX, depthWidth - 1));
    outDepthY = std::max(0, std::min(outDepthY, depthHeight - 1));
}

void FaceDistanceCalculator::UpdateStatistics(double calculationTime, bool wasSuccessful) const {
    m_totalCalculations++;
    
    if (wasSuccessful) {
        m_successfulCalculations++;
    } else {
        m_failedCalculations++;
    }
    
    // Update running average of calculation time
    double alpha = 1.0 / m_totalCalculations;
    m_averageCalculationTime = alpha * calculationTime + (1.0 - alpha) * m_averageCalculationTime;
}

// Convenience functions implementation

std::vector<double> CalculateFaceDistances(const std::vector<cv::Rect>& faces, 
                                         const CameraFrame& frame,
                                         const DistanceCalculationConfig& config) {
    FaceDistanceCalculator calculator(config);
    auto faceDistanceInfos = calculator.CalculateDistances(faces, frame);
    return FaceDistanceCalculator::ExtractDistanceVector(faceDistanceInfos);
}

bool HasFaceInRange(const std::vector<double>& faceDistances, double minDistance, double maxDistance) {
    return std::any_of(faceDistances.begin(), faceDistances.end(),
                      [minDistance, maxDistance](double distance) {
                          return distance >= minDistance && distance <= maxDistance;
                      });
}

int CountFacesInRange(const std::vector<double>& faceDistances, double minDistance, double maxDistance) {
    return static_cast<int>(std::count_if(faceDistances.begin(), faceDistances.end(),
                                        [minDistance, maxDistance](double distance) {
                                            return distance >= minDistance && distance <= maxDistance;
                                        }));
}

} // namespace DualStream