#ifndef EOYS_VIDEO_TO_SPHERE_CV
#define EOYS_VIDEO_TO_SPHERE_CV

#include "al/graphics/al_Shapes.hpp"
#include "al_ext/opencv/al_OpenCV.hpp"
#include <vector>
#include <iostream>
#include <chrono>
#include <memory>

using namespace al;

class VideoSphereLoaderCV {
private:
  Mesh mMesh;
  AlloOpenCV mVideo;
  std::string mVideoFilePath;
  
  // Vector to store all preloaded frames
  std::vector<cv::Mat> mFrames;
  
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
  // Update the displayed frame based on the current frame index
  void updateDisplayFrame() {
    if (mCurrentFrame < 0 || mCurrentFrame >= mFrames.size()) {
      std::cerr << "Invalid frame index: " << mCurrentFrame << std::endl;
      return;
    }
    
    // Make sure we have a valid texture before continuing
    if (!mVideo.videoTexture.created()) {
      std::cerr << "Texture not created in updateDisplayFrame" << std::endl;
      return;
    }
    
    // Make sure the frame is valid
    if (mFrames[mCurrentFrame].empty()) {
      std::cerr << "Empty frame at index " << mCurrentFrame << std::endl;
      return;
    }
    
    try {
      mVideo.videoImage = mFrames[mCurrentFrame];
      mVideo.videoTexture.submit(mVideo.videoImage.ptr());
    } catch (const std::exception& e) {
      std::cerr << "Exception in updateDisplayFrame: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown exception in updateDisplayFrame" << std::endl;
    }
  }

  VideoSphereLoaderCV() {
    mParams << mPlaying << mLooping << mRestarted << mFrameNumber;
    
    // Register a callback for the restart parameter
    mRestarted.registerChangeCallback([this](bool value) {
      if (mFrames.empty()) return; // Don't process if frames aren't loaded yet
      
      mCurrentFrame = 0;
      mCurrentTime = 0.0;
      updateDisplayFrame();
    });
    
    // Register a callback for frame number changes
    mFrameNumber.registerChangeCallback([this](int value) {
      if (mFrames.empty()) return; // Don't process if frames aren't loaded yet
      
      if (value >= 0 && value < mFrameCount) {
        mCurrentFrame = value;
        mCurrentTime = value * mFrameTime;
        updateDisplayFrame();
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
    
    // Preload all frames into memory
    std::cout << "Preloading all " << mFrameCount << " frames..." << std::endl;
    mFrames.clear();
    mFrames.reserve(mFrameCount);
    
    // Set to the beginning of the video
    mVideo.videoCapture->set(cv::CAP_PROP_POS_FRAMES, 0);
    
    // Read all frames
    cv::Mat frame;
    int frameCount = 0;
    
    try {
      while (frameCount < mFrameCount && mVideo.videoCapture->isOpened()) {
        bool success = mVideo.videoCapture->read(frame);
        if (!success || frame.empty()) {
          std::cerr << "Warning: Failed to read frame " << frameCount << std::endl;
          break;
        }
        
        // Create a deep copy of the frame and store it
        cv::Mat frameCopy = frame.clone();
        if (frameCopy.empty()) {
          std::cerr << "Warning: Failed to clone frame " << frameCount << std::endl;
          break;
        }
        
        mFrames.push_back(frameCopy);
        frameCount++;
        
        // Display progress
        if (frameCount % 100 == 0 || frameCount == mFrameCount) {
          std::cout << "  Loaded " << frameCount << " of " << mFrameCount << " frames" << std::endl;
        }
      }
    } catch (const cv::Exception& e) {
      std::cerr << "OpenCV exception while loading frames: " << e.what() << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "Exception while loading frames: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown exception while loading frames" << std::endl;
    }
    
    std::cout << "Preloaded " << mFrames.size() << " frames" << std::endl;
    
    // Verify we got all frames
    if (mFrames.empty()) {
      std::cerr << "Failed to preload frames" << std::endl;
      return false;
    }
    
    // We can now close the video file since we have all frames in memory
    mVideo.cleanupVideoCapture();
    
    // Set the first frame
    if (!mFrames.empty()) {
      mVideo.videoImage = mFrames[0];
      mVideo.videoTexture.submit(mVideo.videoImage.ptr());
      std::cout << "First frame set: " << mVideo.videoImage.cols << "x" << mVideo.videoImage.rows << std::endl;
    }
    
    // Start playback
    mPlaying = true;
    
    return true;
  }
  
  void update(double dt) {
    if (mFrames.empty()) {
      std::cerr << "No frames available in update" << std::endl;
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
        std::cout << "Looping video back to start" << std::endl;
      } else {
        // Stop at the end
        mPlaying = false;
        targetFrame = mFrameCount - 1;
        std::cout << "Reached end of video" << std::endl;
      }
    }
    
    // If we need to jump to a different frame
    if (targetFrame != mCurrentFrame) {
      mCurrentFrame = targetFrame;
      mFrameNumber = targetFrame;
      updateDisplayFrame();
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
    int frame = timeInSeconds * mFrameRate;
    mFrameNumber = frame;
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
    // Clear all preloaded frames to free memory
    mFrames.clear();
    mVideo.cleanupVideoCapture();
    mPlaying = false;
  }
};

#endif // EOYS_VIDEO_TO_SPHERE_CV
