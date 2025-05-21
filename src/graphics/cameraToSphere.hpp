#ifndef EOYS_CAMERA_TO_SPHERE
#define EOYS_CAMERA_TO_SPHERE

#include "al/graphics/al_Shapes.hpp"
#include "al_ext/opencv/al_OpenCV.hpp"
#include <iostream>

using namespace al;

class CameraSphereLoader {
private:
  Mesh mMesh;
  AlloOpenCV mCamera;
  
  // Camera properties
  int mCameraWidth = 0;
  int mCameraHeight = 0;
  double aspectRatio = 1.0;
  
  // Camera control
  al::ParameterBool mCameraActive {"mCameraActive", "", true};
  al::ParameterInt mCameraDevice {"mCameraDevice", "", 0, 0, 10};
  al::ParameterBundle mParams{"CameraSphereLoader"};

public:
  CameraSphereLoader() {
    mParams << mCameraActive << mCameraDevice;
    
    // Register callback for camera device change
    mCameraDevice.registerChangeCallback([this](int value) {
      if (mCameraActive) {
        this->cleanup();
        this->initCamera(value);
      }
    });
  }

  ~CameraSphereLoader() {
    this->cleanup();
  }

  al::ParameterBundle& params() {
    return this->mParams;
  }
  
  bool initCamera(int deviceIndex = 0) {
    // Create a sphere mesh for rendering
    addTexSphere(mMesh, 1.0, 24, true);
    
    // Initialize the camera device
    mCamera.initializeVideoCaptureDevice(deviceIndex, true);
    
    if (!mCamera.videoCapture || !mCamera.videoCapture->isOpened()) {
      std::cerr << "Error opening camera device " << deviceIndex << std::endl;
      return false;
    }
    
    // Get camera dimensions
    mCameraWidth = mCamera.videoCapture->get(cv::CAP_PROP_FRAME_WIDTH);
    mCameraHeight = mCamera.videoCapture->get(cv::CAP_PROP_FRAME_HEIGHT);
    aspectRatio = mCameraWidth / (double)mCameraHeight;
    
    mCameraActive = true;
    
    return true;
  }
  
  void update(double dt) {
    if (!mCameraActive) return;
    
    if (mCamera.videoCapture && mCamera.videoCapture->isOpened()) {
      mCamera.captureFrame(); // Capture the frame
      
      if (!mCamera.videoImage.empty()) {
        // Only submit if we got a valid frame
        mCamera.videoTexture.submit(mCamera.videoImage.ptr());
      }
    }
  }

  void draw(Graphics& g) {
    if (!mCameraActive) return;
    
    g.pushMatrix();
    mCamera.videoTexture.bind(0);
    g.texture();
    g.draw(mMesh);
    mCamera.videoTexture.unbind(0);
    g.popMatrix();
  }
  
  // Start the camera
  void startCamera() { mCameraActive = true; }

  // Stop the camera
  void stopCamera() { mCameraActive = false; }

  // Toggle camera
  void toggleCamera() { mCameraActive = !mCameraActive; }
  
  // Switch camera device
  void setDevice(int deviceIndex) { mCameraDevice = deviceIndex; }
  
  // Getters for camera properties
  double getAspectRatio() const { return aspectRatio; }
  
  void cleanup() {
    mCamera.cleanupVideoCapture();
    mCameraActive = false;
  }
};

#endif // EOYS_CAMERA_TO_SPHERE
