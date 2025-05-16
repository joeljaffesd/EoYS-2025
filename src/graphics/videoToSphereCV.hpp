#ifndef EOYS_VIDEO_TO_SPHERE_CV
#define EOYS_VIDEO_TO_SPHERE_CV

#include "al/graphics/al_Shapes.hpp"
#include "al_ext/opencv/al_OpenCV.hpp"
#include <vector>
#include <iostream>
#include <chrono>

using namespace al;

class VideoSphereLoaderCV {
private:
  Mesh mMesh;
  AlloOpenCV mVideo;
  std::string mVideoFilePath;
  
  // Video properties
  int mVideoWidth = 0;
  int mVideoHeight = 0;
  double aspectRatio = 1.0;
  double mFrameRate = 30.0;
  double mFrameTime = 1.0 / 30.0;
  double mCurrentTime = 0.0;
  int mFrameCount = 0;
  int mCurrentFrame = 0;
  
  // Playback control
  al::ParameterBool mPlaying {"mPlaying", "", false};
  al::ParameterBool mLooping {"mLooping", "", false};
  al::ParameterBool mRestarted {"mRestart", "", false};
  al::ParameterInt mFrameNumber {"mFrameNumber", "", 0, 0, std::numeric_limits<int>::max()};
  al::ParameterBundle mParams{"VideoSphereLoaderCV"};

public:
  VideoSphereLoaderCV() {
    mParams << mPlaying << mLooping << mRestarted << mFrameNumber;
    
    // Register a callback for the restart parameter
    mRestarted.registerChangeCallback([this](bool value) {
      if (mVideo.videoCapture) {
        mVideo.videoCapture->set(cv::CAP_PROP_POS_FRAMES, 0);
        mCurrentFrame = 0;
        mCurrentTime = 0.0;
      }
    });
    
    // Register a callback for frame number changes
    mFrameNumber.registerChangeCallback([this](int value) {
      if (mVideo.videoCapture && value >= 0 && value < mFrameCount) {
        mVideo.videoCapture->set(cv::CAP_PROP_POS_FRAMES, value);
        mCurrentFrame = value;
        mCurrentTime = value * mFrameTime;
      }
    });
  }

  ~VideoSphereLoaderCV() {
    this->cleanup();
  }

  al::ParameterBundle& params() {
    return this->mParams;
  }
  
  bool loadVideo(const std::string& videoFilePath) {
    std::cout << "Loading video: " << videoFilePath << std::endl;
    // Create a sphere mesh for rendering
    addTexSphere(mMesh, 15.0, 24, true);
    mVideoFilePath = videoFilePath;
    
    // Initialize the video file
    mVideo.initializeVideoCaptureFile(mVideoFilePath, true);
    
    if (!mVideo.videoCapture || !mVideo.videoCapture->isOpened()) {
      std::cerr << "Error opening video file: " << mVideoFilePath << std::endl;
      return false;
    }
    
    // Get video properties
    mVideoWidth = mVideo.videoCapture->get(cv::CAP_PROP_FRAME_WIDTH);
    mVideoHeight = mVideo.videoCapture->get(cv::CAP_PROP_FRAME_HEIGHT);
    mFrameRate = mVideo.videoCapture->get(cv::CAP_PROP_FPS);
    mFrameTime = 1.0 / mFrameRate;
    mFrameCount = mVideo.videoCapture->get(cv::CAP_PROP_FRAME_COUNT);
    aspectRatio = mVideoWidth / (double)mVideoHeight;
    
    std::cout << "Video properties:" << std::endl;
    std::cout << "  Dimensions: " << mVideoWidth << "x" << mVideoHeight << std::endl;
    std::cout << "  Frame rate: " << mFrameRate << " fps" << std::endl;
    std::cout << "  Frame count: " << mFrameCount << std::endl;
    std::cout << "  Duration: " << mFrameCount / mFrameRate << " seconds" << std::endl;
    
    // Configure texture
    mVideo.videoTexture.filter(Texture::LINEAR);
    mVideo.videoTexture.wrap(Texture::REPEAT, Texture::CLAMP_TO_EDGE, Texture::CLAMP_TO_EDGE);
    
    // Capture the first frame
    mVideo.captureFrame();
    
    if (mVideo.videoImage.empty()) {
      std::cerr << "Failed to capture first frame" << std::endl;
      return false;
    }
    
    std::cout << "First frame captured: " << mVideo.videoImage.cols << "x" << mVideo.videoImage.rows << std::endl;
    
    // Submit the first frame to the texture
    mVideo.videoTexture.submit(mVideo.videoImage.ptr());
    
    // Start playback
    mPlaying = true;
    
    return true;
  }
  
  void update(double dt) {
    if (!mVideo.videoCapture || !mVideo.videoCapture->isOpened()) {
      std::cerr << "Video capture not opened in update" << std::endl;
      return;
    }
    
    // Handle pause state
    if (!mPlaying) return;
    
    // Update current time
    mCurrentTime += dt;
    
    // Calculate the frame to display based on current time
    int targetFrame = mCurrentTime * mFrameRate;
    
    // Check if we need to loop
    if (targetFrame >= mFrameCount) {
      if (mLooping) {
        // Loop back to the beginning
        mCurrentTime = 0;
        targetFrame = 0;
        mVideo.videoCapture->set(cv::CAP_PROP_POS_FRAMES, 0);
        std::cout << "Looping video back to start" << std::endl;
      } else {
        // Stop at the end
        mPlaying = false;
        targetFrame = mFrameCount - 1;
        mVideo.videoCapture->set(cv::CAP_PROP_POS_FRAMES, targetFrame);
        std::cout << "Reached end of video" << std::endl;
      }
    }
    
    // If we need to jump to a different frame
    if (targetFrame != mCurrentFrame) {
      mVideo.videoCapture->set(cv::CAP_PROP_POS_FRAMES, targetFrame);
      mCurrentFrame = targetFrame;
      mFrameNumber = targetFrame;
    }
    
    // Capture the current frame
    mVideo.captureFrame();
    
    // Check if we got a valid frame
    if (mVideo.videoImage.empty()) {
      std::cerr << "Empty frame captured at time " << mCurrentTime << ", frame " << mCurrentFrame << std::endl;
    } else {
      // Submit to texture
      mVideo.videoTexture.submit(mVideo.videoImage.ptr());
    }
  }

  void draw(Graphics& g) {
    if (!mVideo.videoTexture.created()) {
      std::cerr << "Texture not created in draw" << std::endl;
      return;
    }
    
    g.pushMatrix();
    mVideo.videoTexture.bind(0);
    g.texture();
    g.draw(mMesh);
    mVideo.videoTexture.unbind(0);
    g.popMatrix();
  }
  
  // Play
  void play() { mPlaying = true; }

  // Pause
  void pause() { mPlaying = false; }

  // Toggle play/pause
  void togglePlayPause() { mPlaying = !mPlaying; }

  // Restart the video
  void restart() { mRestarted = !mRestarted; }

  // Toggle looping
  void toggleLooping() { mLooping = !mLooping; }

  // Seek to a specific time (in seconds)
  void seek(double timeInSeconds) {
    if (mVideo.videoCapture) {
      int frame = timeInSeconds * mFrameRate;
      mFrameNumber = frame;
    }
  }
  
  // Seek to a specific frame
  void seekFrame(int frame) {
    mFrameNumber = frame;
  }
  
  // Getters for video properties
  double getAspectRatio() const { return aspectRatio; }
  double getDuration() const { return mFrameCount / mFrameRate; }
  int getFrameCount() const { return mFrameCount; }
  double getCurrentTime() const { return mCurrentTime; }
  int getCurrentFrame() const { return mCurrentFrame; }
  
  void cleanup() {
    mVideo.cleanupVideoCapture();
    mPlaying = false;
  }
};

#endif // EOYS_VIDEO_TO_SPHERE_CV
