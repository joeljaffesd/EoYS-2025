#ifndef EOYS_VIDEO_TO_SPHERE_CV
#define EOYS_VIDEO_TO_SPHERE_CV

#include "al/graphics/al_Shapes.hpp"
#include "al_ext/opencv/al_OpenCV.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include <vector>
#include <iostream>
#include <chrono>
#include <memory>

class VideoSphereLoaderCV : public al::PositionedVoice {
private:
  al::Mesh mMesh;
  al::AlloOpenCV mVideo;
  std::string mVideoFilePath;
  al::ControlGUI mGUI;
  
  // Vector to store all preloaded frames
  std::vector<cv::Mat> mFrames;
  
  // Video properties
  int mVideoWidth = 0;
  int mVideoHeight = 0;
  double aspectRatio = 1.0;
  double mFrameRate = 30.0;
  double mFrameTime = 1.0 / 30.0;
  int mFrameCount = 0;
  int mCurrentFrame = 0;
  
  // Playback control
  al::ParameterBool mPlaying {"mPlaying", "", false};
  al::ParameterBool mLooping {"mLooping", "", true};
  al::ParameterBool mRestarted {"mRestarted", "", false};
  bool restartFlag = false; // hack for networked restart
  al::Parameter mCurrentTime {"mCurrentTime", "", 0, 0, std::numeric_limits<int>::max()};
  al::ParameterBundle mParams{"VideoSphereLoaderCV"};
  bool initFlag = true;

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
      mVideo.videoTexture.submit(mVideo.videoImage.ptr()); // seg faults here
    } catch (const std::exception& e) {
      std::cerr << "Exception in updateDisplayFrame: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown exception in updateDisplayFrame" << std::endl;
    }
  }

  VideoSphereLoaderCV() {
    mParams << mPlaying << mLooping << mRestarted << mCurrentTime;
  }

  ~VideoSphereLoaderCV() {
    this->cleanup();
  }

  al::ParameterBundle& params() {
    return this->mParams;
  }

  void init() override {
    // plz tell me there's a better way to do this
    for (auto& param : mParams.parameters()) {
      auto pp = static_cast<al::Parameter*>(param);
      this->registerParameter(*pp);
    }

    mGUI.registerParameterBundle(this->params());
    //this->loadVideo(); // moved to flag
  }
  
  bool loadVideo(const std::string& videoFilePath = "../assets/videos/vid.mp4") {
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
    mVideo.videoTexture.filter(al::Texture::LINEAR);
    mVideo.videoTexture.wrap(al::Texture::REPEAT, al::Texture::CLAMP_TO_EDGE, al::Texture::CLAMP_TO_EDGE);
    
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
  
  void update(double dt) override {
    if (mFrames.empty()) {
      std::cerr << "No frames available in update" << std::endl;
      return;
    }
    
    // Handle pause state
    if (!mPlaying) return;

    if (mRestarted != restartFlag) {
      mCurrentFrame = 0;
      mCurrentTime = 0.0;
      updateDisplayFrame();
      restartFlag = !restartFlag;
    }
    
    // Update current time
    mCurrentTime = mCurrentTime + dt;
    
    // Calculate the frame to display based on current time
    int targetFrame = mCurrentTime * mFrameRate;
    
    // Check if we need to loop
    if (targetFrame >= mFrameCount) {
      if (mLooping) {
        // Loop back to the beginning
        mCurrentTime = 0;
        targetFrame = 0;
        // std::cout << "Looping video back to start" << std::endl;
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
      //mFrameNumber = targetFrame;
      updateDisplayFrame();
    }
  }

  void onProcess(al::Graphics& g) {
    if (this->initFlag) {
      this->loadVideo();
      this->initFlag = false;
    }

    
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
    if (!mIsReplica) {
      mGUI.draw(g);
    }
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

  // not networked yet
  // // Seek to a specific time (in seconds)
  // void seek(double timeInSeconds) {
  //   int frame = timeInSeconds * mFrameRate;
  //   //mFrameNumber = frame;
  // }
  
  // not networked yet
  // // Seek to a specific frame
  // void seekFrame(int frame) {
  //   //mFrameNumber = frame;
  // }
  
  // Getters for video properties
  double getAspectRatio() const { return aspectRatio; }
  double getDuration() const { return mFrameCount / mFrameRate; }
  int getFrameCount() const { return mFrameCount; }
  //double getCurrentTime() const { return mCurrentTime; }
  int getCurrentFrame() const { return mCurrentFrame; }
  
  void cleanup() {
    // Clear all preloaded frames to free memory
    mFrames.clear();
    mVideo.cleanupVideoCapture();
    mPlaying = false;
  }
};

#endif // EOYS_VIDEO_TO_SPHERE_CV
