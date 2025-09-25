#pragma once

#include <chrono>
#include <unordered_map>
#include <vector>

/**
 * @file OpticalParameters.h
 * @brief Optical parameter system for Light Field Display (LFD) integration
 * 
 * This system provides optical parameter management for different viewing modes,
 * integrating the original streamer's PX/PO calculations with the dual-stream architecture.
 * Supports 3-stream video architecture for plane, stereo, and multiview modes.
 */

namespace DualStream {

/**
 * @brief Viewing modes for Light Field Display
 * 
 * These modes determine which video stream to display and which optical parameters to use:
 * - planeMode: Single close-up view (Video 1: plane.mp4)
 * - stereoMode: Dual stereo view (Video 2: stereo.mp4) 
 * - multiviewMode: Multi-angle view (Video 3: multiview.mp4)
 */
enum class ViewMode {
    PlaneMode,      ///< Single face close-up (< 500mm)
    StereoMode,     ///< Single face optimal distance (500-2500mm)
    MultiviewMode   ///< No face or far face (>= 2500mm) or multiple faces
};

/**
 * @brief Optical parameters for Light Field Display rendering
 * 
 * PX (pixel pitch) and PO (pixel offset) are critical parameters for proper
 * light field display rendering. These values are calculated from eye tracking
 * and face position data to provide optimal viewing experience.
 */
struct OpticalParams {
    double PX = 0.0;        ///< Pixel pitch parameter
    double PO = 0.0;        ///< Pixel offset parameter
    bool isValid = false;   ///< Whether parameters are valid/calculated
    
    OpticalParams() = default;
    OpticalParams(double px, double po) : PX(px), PO(po), isValid(true) {}
    
    void Reset() {
        PX = 0.0;
        PO = 0.0;
        isValid = false;
    }
};

/**
 * @brief Enhanced optical parameters for Light Field Display rendering
 * 
 * Extended optical parameters that include view mode and additional
 * parameters for sophisticated light field display rendering.
 */
struct OpticalParameters {
    ViewMode viewMode = ViewMode::MultiviewMode;  ///< Current view mode
    float px = 0.0f;       ///< Pixel pitch X
    float py = 0.0f;       ///< Pixel pitch Y
    float pox = 0.0f;      ///< Pixel offset X
    float poy = 0.0f;      ///< Pixel offset Y
    float confidence = 0.0f; ///< Confidence of optical parameters
    
    OpticalParameters() = default;
    OpticalParameters(ViewMode mode, float pixelPitchX, float pixelPitchY, 
                     float pixelOffsetX = 0.0f, float pixelOffsetY = 0.0f, float conf = 1.0f)
        : viewMode(mode), px(pixelPitchX), py(pixelPitchY), 
          pox(pixelOffsetX), poy(pixelOffsetY), confidence(conf) {}
    
    void Reset() {
        viewMode = ViewMode::MultiviewMode;
        px = py = pox = poy = confidence = 0.0f;
    }
    
    bool IsValid() const {
        return confidence > 0.0f;
    }
};

/**
 * @brief Configuration for optical parameter calculation
 */
struct OpticalParametersConfig {
    // Default optical parameters for each mode
    OpticalParams defaultPlaneParams{0.5, 0.0};      ///< Default for plane mode
    OpticalParams defaultStereoParams{1.0, 0.0};     ///< Default for stereo mode  
    OpticalParams defaultMultiviewParams{1.5, 0.0};  ///< Default for multiview mode
    
    // Calculation parameters
    bool enableAdaptiveCalculation = true;    ///< Enable real-time parameter calculation
    bool useDefaultForNonStereo = true;       ///< Use defaults for plane/multiview modes
    double calculationSmoothingFactor = 0.1;  ///< Smoothing for parameter transitions
    
    // Integration with rendering
    bool enableUniformUpdates = true;         ///< Automatically update shader uniforms
    std::string pxUniformName = "u_pixelPitch";    ///< Shader uniform name for PX
    std::string poUniformName = "u_pixelOffset";   ///< Shader uniform name for PO
};

/**
 * @brief Manages optical parameters for Light Field Display integration
 * 
 * This class bridges the original streamer's optical parameter calculation
 * with the dual-stream rendering system. It provides:
 * - ViewMode-based parameter selection
 * - Real-time parameter calculation from face tracking data
 * - Integration with rendering system for shader uniforms
 * - 3-stream video architecture support
 */
class OpticalParametersManager {
public:
    explicit OpticalParametersManager(const OpticalParametersConfig& config = OpticalParametersConfig());
    ~OpticalParametersManager() = default;
    
    // Core functionality
    
    /**
     * @brief Update optical parameters based on current viewing mode and face data
     * @param mode Current viewing mode
     * @param faceDistance Distance to primary face in mm (if available)
     * @param facePosition Face position data for calculation (if available)
     * @return Updated optical parameters
     */
    OpticalParams UpdateParameters(ViewMode mode, double faceDistance = -1.0, 
                                 const std::pair<double, double>& facePosition = {0.0, 0.0});
    
    /**
     * @brief Get current optical parameters
     * @return Current optical parameters
     */
    const OpticalParams& GetCurrentParameters() const { return m_currentParams; }
    
    /**
     * @brief Get optical parameters for specific viewing mode
     * @param mode Viewing mode
     * @param useCalculated Whether to use calculated values (if available) or defaults
     * @return Optical parameters for the mode
     */
    OpticalParams GetParametersForMode(ViewMode mode, bool useCalculated = true) const;
    
    /**
     * @brief Get current viewing mode
     * @return Current viewing mode
     */
    ViewMode GetCurrentMode() const { return m_currentMode; }
    
    /**
     * @brief Check if current parameters are calculated (not default)
     * @return True if parameters are calculated from real face data
     */
    bool AreParametersCalculated() const { return m_currentParams.isValid && m_hasCalculatedParams; }
    
    // Configuration management
    
    /**
     * @brief Update configuration
     * @param config New configuration
     */
    void UpdateConfig(const OpticalParametersConfig& config);
    
    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    const OpticalParametersConfig& GetConfig() const { return m_config; }
    
    // Video stream mapping
    
    /**
     * @brief Get video stream index for viewing mode (for 3-stream architecture)
     * @param mode Viewing mode
     * @return Video stream index (0=plane, 1=stereo, 2=multiview)
     */
    static int GetVideoStreamIndex(ViewMode mode);
    
    /**
     * @brief Get recommended video file name for viewing mode
     * @param mode Viewing mode
     * @return Recommended file name
     */
    static std::string GetVideoFileName(ViewMode mode);
    
    // Utility functions
    
    /**
     * @brief Convert ViewMode to string
     * @param mode Viewing mode
     * @return String representation
     */
    static std::string ViewModeToString(ViewMode mode);
    
    /**
     * @brief Convert string to ViewMode
     * @param modeStr String representation
     * @return Viewing mode
     */
    static ViewMode StringToViewMode(const std::string& modeStr);
    
    /**
     * @brief Reset to default state
     */
    void Reset();

private:
    // Configuration
    OpticalParametersConfig m_config;
    
    // Current state
    ViewMode m_currentMode = ViewMode::MultiviewMode;
    OpticalParams m_currentParams;
    bool m_hasCalculatedParams = false;
    
    // Calculated parameters cache
    std::unordered_map<ViewMode, OpticalParams> m_calculatedParams;
    
    // Smoothing and history
    std::vector<OpticalParams> m_parameterHistory;
    static constexpr size_t MAX_HISTORY_SIZE = 10;
    
    // Private methods
    
    /**
     * @brief Calculate optical parameters from face data
     * @param faceDistance Distance to face in mm
     * @param facePosition Face position (x, y) in normalized coordinates
     * @return Calculated optical parameters
     */
    OpticalParams CalculateParametersFromFace(double faceDistance, 
                                            const std::pair<double, double>& facePosition) const;
    
    /**
     * @brief Apply smoothing to parameter transitions
     * @param newParams New parameters
     * @return Smoothed parameters
     */
    OpticalParams ApplySmoothing(const OpticalParams& newParams) const;
    
    /**
     * @brief Choose between default and calculated parameters based on mode
     * @param mode Viewing mode
     * @return Appropriate optical parameters
     */
    OpticalParams ChooseParametersForMode(ViewMode mode) const;
    
    /**
     * @brief Update parameter history for smoothing
     * @param params New parameters
     */
    void UpdateParameterHistory(const OpticalParams& params);
};

// Convenience functions for integration with existing dual-stream code

/**
 * @brief Determine viewing mode from face count and distances
 * @param faceCount Number of detected faces
 * @param faceDistances Vector of face distances in mm
 * @return Appropriate viewing mode
 */
ViewMode DetermineViewModeFromFaces(int faceCount, const std::vector<double>& faceDistances);

/**
 * @brief Get number of video streams for viewing mode
 * @param mode Viewing mode
 * @return Number of views (1 for plane, 2 for stereo, 10+ for multiview)
 */
int GetNumViewsForMode(ViewMode mode);

/**
 * @brief Check if mode should use default optical parameters
 * @param mode Viewing mode
 * @return True if mode should use defaults (plane/multiview), false for calculated (stereo)
 */
bool ShouldUseDefaultParameters(ViewMode mode);

} // namespace DualStream