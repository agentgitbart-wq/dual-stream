#include "OpticalParameters.h"
#include "OpticalParameters.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace DualStream {

// OpticalParametersManager Implementation

OpticalParametersManager::OpticalParametersManager(const OpticalParametersConfig& config)
    : m_config(config) {
    Reset();
}

OpticalParams OpticalParametersManager::UpdateParameters(ViewMode mode, double faceDistance, 
                                                        const std::pair<double, double>& facePosition) {
    m_currentMode = mode;
    
    OpticalParams newParams;
    
    // For stereo mode with valid face data, calculate parameters
    if (mode == ViewMode::StereoMode && faceDistance > 0 && m_config.enableAdaptiveCalculation) {
        newParams = CalculateParametersFromFace(faceDistance, facePosition);
        m_calculatedParams[mode] = newParams;
        m_hasCalculatedParams = true;
        
        LOG_DEBUG("Calculated optical parameters: PX={:.3f}, PO={:.3f} for distance={:.1f}mm", 
                 newParams.PX, newParams.PO, faceDistance);
    } else {
        // Use appropriate parameters for the mode
        newParams = ChooseParametersForMode(mode);
    }
    
    // Apply smoothing if enabled
    if (m_config.calculationSmoothingFactor > 0.0) {
        newParams = ApplySmoothing(newParams);
    }
    
    // Update current parameters
    m_currentParams = newParams;
    UpdateParameterHistory(newParams);
    
    LOG_TRACE("Updated optical parameters: Mode={}, PX={:.3f}, PO={:.3f}", 
             ViewModeToString(mode), m_currentParams.PX, m_currentParams.PO);
    
    return m_currentParams;
}

OpticalParams OpticalParametersManager::GetParametersForMode(ViewMode mode, bool useCalculated) const {
    if (useCalculated && m_calculatedParams.find(mode) != m_calculatedParams.end()) {
        return m_calculatedParams.at(mode);
    }
    
    return ChooseParametersForMode(mode);
}

void OpticalParametersManager::UpdateConfig(const OpticalParametersConfig& config) {
    m_config = config;
    LOG_INFO("Updated optical parameters configuration");
}

int OpticalParametersManager::GetVideoStreamIndex(ViewMode mode) {
    switch (mode) {
        case ViewMode::PlaneMode:     return 0;  // Video 1: plane.mp4
        case ViewMode::StereoMode:    return 1;  // Video 2: stereo.mp4
        case ViewMode::MultiviewMode: return 2;  // Video 3: multiview.mp4
        default:                      return 2;  // Default to multiview
    }
}

std::string OpticalParametersManager::GetVideoFileName(ViewMode mode) {
    switch (mode) {
        case ViewMode::PlaneMode:     return "plane.mp4";
        case ViewMode::StereoMode:    return "stereo.mp4";
        case ViewMode::MultiviewMode: return "multiview.mp4";
        default:                      return "multiview.mp4";
    }
}

std::string OpticalParametersManager::ViewModeToString(ViewMode mode) {
    switch (mode) {
        case ViewMode::PlaneMode:     return "PlaneMode";
        case ViewMode::StereoMode:    return "StereoMode";
        case ViewMode::MultiviewMode: return "MultiviewMode";
        default:                      return "Unknown";
    }
}

ViewMode OpticalParametersManager::StringToViewMode(const std::string& modeStr) {
    if (modeStr == "PlaneMode" || modeStr == "plane") {
        return ViewMode::PlaneMode;
    } else if (modeStr == "StereoMode" || modeStr == "stereo") {
        return ViewMode::StereoMode;
    } else if (modeStr == "MultiviewMode" || modeStr == "multiview") {
        return ViewMode::MultiviewMode;
    } else {
        LOG_WARNING("Unknown view mode string: '{}', defaulting to MultiviewMode", modeStr);
        return ViewMode::MultiviewMode;
    }
}

void OpticalParametersManager::Reset() {
    m_currentMode = ViewMode::MultiviewMode;
    m_currentParams = m_config.defaultMultiviewParams;
    m_hasCalculatedParams = false;
    m_calculatedParams.clear();
    m_parameterHistory.clear();
    
    LOG_DEBUG("Reset optical parameters manager to default state");
}

// Private method implementations

OpticalParams OpticalParametersManager::CalculateParametersFromFace(double faceDistance, 
                                                                   const std::pair<double, double>& facePosition) const {
    // Implementation based on original streamer's PX/PO calculation logic
    // This is a simplified version - the actual calculation would depend on 
    // camera calibration parameters and LFD specifications
    
    OpticalParams params;
    
    // Distance-based PX calculation (pixel pitch)
    // Closer faces need smaller pixel pitch for proper rendering
    const double basePX = 1.0;
    const double distanceNorm = std::max(500.0, std::min(2500.0, faceDistance)); // Clamp to stereo range
    params.PX = basePX * (distanceNorm / 1500.0); // Normalize around 1500mm optimal distance
    
    // Position-based PO calculation (pixel offset)
    // Face position affects viewing angle offset
    const double basePO = 0.0;
    const double maxOffset = 0.5;
    params.PO = basePO + (facePosition.first * maxOffset); // Use X position for offset
    
    // Ensure reasonable bounds
    params.PX = std::max(0.1, std::min(3.0, params.PX));
    params.PO = std::max(-1.0, std::min(1.0, params.PO));
    
    params.isValid = true;
    
    return params;
}

OpticalParams OpticalParametersManager::ApplySmoothing(const OpticalParams& newParams) const {
    if (m_parameterHistory.empty() || m_config.calculationSmoothingFactor <= 0.0) {
        return newParams;
    }
    
    const OpticalParams& lastParams = m_parameterHistory.back();
    const double alpha = m_config.calculationSmoothingFactor;
    
    OpticalParams smoothed;
    smoothed.PX = lastParams.PX * (1.0 - alpha) + newParams.PX * alpha;
    smoothed.PO = lastParams.PO * (1.0 - alpha) + newParams.PO * alpha;
    smoothed.isValid = newParams.isValid;
    
    return smoothed;
}

OpticalParams OpticalParametersManager::ChooseParametersForMode(ViewMode mode) const {
    // Use default parameters based on mode, following original streamer logic
    switch (mode) {
        case ViewMode::PlaneMode:
            return m_config.defaultPlaneParams;
        case ViewMode::StereoMode:
            // For stereo mode, prefer calculated parameters if available
            if (m_hasCalculatedParams && m_calculatedParams.find(mode) != m_calculatedParams.end() && 
                !m_config.useDefaultForNonStereo) {
                return m_calculatedParams.at(mode);
            }
            return m_config.defaultStereoParams;
        case ViewMode::MultiviewMode:
        default:
            return m_config.defaultMultiviewParams;
    }
}

void OpticalParametersManager::UpdateParameterHistory(const OpticalParams& params) {
    m_parameterHistory.push_back(params);
    
    // Maintain history size limit
    if (m_parameterHistory.size() > MAX_HISTORY_SIZE) {
        m_parameterHistory.erase(m_parameterHistory.begin());
    }
}

// Convenience functions implementation

ViewMode DetermineViewModeFromFaces(int faceCount, const std::vector<double>& faceDistances) {
    // Implementation of original streamer's GetSingleFrameMode logic
    switch (faceCount) {
        case 0:
            return ViewMode::MultiviewMode;
            
        case 1: {
            if (faceDistances.empty()) {
                return ViewMode::MultiviewMode;
            }
            
            double distance = faceDistances[0];
            if (distance < 500.0) {
                return ViewMode::PlaneMode;
            } else if (distance >= 2500.0) {
                return ViewMode::MultiviewMode;
            } else {
                return ViewMode::StereoMode; // 500-2500mm range
            }
        }
        
        default: { // Multiple faces
            // Check if any face is >= 1000mm
            for (double distance : faceDistances) {
                if (distance >= 1000.0) {
                    return ViewMode::MultiviewMode;
                }
            }
            return ViewMode::PlaneMode;
        }
    }
}

int GetNumViewsForMode(ViewMode mode) {
    // Based on original streamer's ChooseNumViews function
    switch (mode) {
        case ViewMode::PlaneMode:     return 1;   // Single view
        case ViewMode::StereoMode:    return 2;   // Stereo pair
        case ViewMode::MultiviewMode: return 10;  // Multi-angle array
        default:                      return 10;
    }
}

bool ShouldUseDefaultParameters(ViewMode mode) {
    // Based on original streamer's ShouldChooseDefaultParams function
    return mode != ViewMode::StereoMode;
}

} // namespace DualStream