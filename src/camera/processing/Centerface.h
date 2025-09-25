#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

/**
 * @brief FaceInfo structure matching original EyeTrackingDLL implementation
 * Contains bounding box, confidence score, and facial landmarks
 */
typedef struct FaceInfo {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    float landmarks[10];  // 5 facial landmarks (x,y pairs)
} FaceInfo;

/**
 * @brief Original CenterFace detector from EyeTrackingDLL
 * 
 * This is the exact implementation used in the original Samsung LFD streamer.
 * Supports facial landmark detection and provides the same API as the original.
 * 
 * Usage example (exactly like original EyeTrackingDLL):
 * ```cpp
 * Centerface centerface("centerface.onnx", 640, 480);
 * centerface.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
 * centerface.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
 * 
 * cv::Mat image;
 * std::vector<FaceInfo> faces;
 * centerface.detect(image, faces, 0.7f, 0.3f);
 * 
 * for (const auto& face : faces) {
 *     cv::rectangle(image, cv::Point(face.x1, face.y1), 
 *                          cv::Point(face.x2, face.y2), cv::Scalar(0, 255, 0), 2);
 *     // Draw landmarks
 *     for (int i = 0; i < 5; i++) {
 *         cv::circle(image, cv::Point(face.landmarks[i*2], face.landmarks[i*2+1]), 2, cv::Scalar(0, 0, 255), -1);
 *     }
 * }
 * ```
 */
class Centerface {
public:
    /**
     * @brief Constructor matching original EyeTrackingDLL API
     * @param model_path Path to centerface.onnx model
     * @param width Input width for the model
     * @param height Input height for the model
     */
    Centerface(std::string model_path, int width, int height);
    
    /**
     * @brief Destructor
     */
    ~Centerface();
    
    /**
     * @brief Detect faces in image (original EyeTrackingDLL API)
     * @param image Input image (BGR format) - will be modified
     * @param faces Output vector of detected faces with landmarks
     * @param scoreThresh Score threshold for detection (default 0.5)
     * @param nmsThresh NMS threshold for overlapping faces (default 0.3)
     */
    void detect(cv::Mat& image, std::vector<FaceInfo>& faces, float scoreThresh = 0.5, float nmsThresh = 0.3);
    
    /**
     * @brief Set preferred DNN backend
     * @param backend OpenCV DNN backend (e.g., DNN_BACKEND_CUDA)
     */
    void setPreferableBackend(cv::dnn::Backend backend);
    
    /**
     * @brief Set preferred DNN target
     * @param target OpenCV DNN target (e.g., DNN_TARGET_CUDA)
     */
    void setPreferableTarget(cv::dnn::Target target);
    
    /**
     * @brief Check if detector is ready
     * @return True if model is loaded and ready
     */
    bool isReady() const;

private:
    // Internal processing methods
    void nms(std::vector<FaceInfo>& input, std::vector<FaceInfo>& output, float nmsthreshold = 0.3);
    void decode(cv::Mat& heatmap, cv::Mat& scale, cv::Mat& offset, cv::Mat& landmarks, std::vector<FaceInfo>& faces, float scoreThresh, float nmsThresh);
    void dynamic_scale(float in_w, float in_h);
    std::vector<int> getIds(float* heatmap, int h, int w, float thresh);
    void squareBox(std::vector<FaceInfo>& faces);

private:
    // Dynamic scaling parameters
    int d_h;
    int d_w;
    float d_scale_h;
    float d_scale_w;

    // Image scaling factors
    float scale_w;
    float scale_h;

    // Current image dimensions
    int image_h;
    int image_w;

    // OpenCV DNN network
    cv::dnn::Net net;
};