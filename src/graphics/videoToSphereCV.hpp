#ifndef EOYS_VIDEO_TO_SPHERE_CV
#define EOYS_VIDEO_TO_SPHERE_CV

#include "al/graphics/al_Shapes.hpp"
#include "al_ext/opencv/al_OpenCV.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include <vector>
#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <mutex>

class VideoSphereLoaderCV : public al::PositionedVoice {
private:
  al::Mesh mMesh;
  al::AlloOpenCV mVideo;
  al::ParameterString mVideoFilePath {"mVideoFilePath", "", "../assets/videos/sky.mp4"};
  al::ControlGUI mGUI;
  
  // Vector to store all preloaded frames
  std::vector<cv::Mat> mFrames;
  
  // Async loading variables
  std::thread mLoadingThread;
  std::mutex mFramesMutex;
  bool mIsLoading = false;
  
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

  al::ParameterBool networkedInitFlag {"networkedInitFlag", "", true};
  bool initFlag = true;

  float speed = 1.f;

  al::ParameterBool rotate{"rotate", "", false};
  al::Parameter rotation{"rotation", "", 0.f, 0.f, 360.f};
  al::Parameter rotationSpeed{"rotationSpeed", "", 0.01f, 0.f, 1.f}; 

public:

  void setSpeed(float newSpeed) {
    this->speed = newSpeed;
  }

  void toggleRotation(bool value) {
    rotate.set(value);
    std::cout << "Rotation toggled to: " << (value ? "true" : "false") << std::endl;
  }

  void setRotationSpeed(float speed) {
    rotationSpeed.set(speed);
    std::cout << "Rotation speed set to: " << speed << std::endl;
  }

  void setVideoFilePath(const std::string& videoFilePath) {
    mVideoFilePath.set(videoFilePath);
    // this->initFlag = true;
    this->networkedInitFlag = !networkedInitFlag;
  }

  // Update the displayed frame based on the current frame index
  void updateDisplayFrame() {
    std::lock_guard<std::mutex> lock(mFramesMutex);
    
    if (mFrames.empty()) {
      std::cerr << "No frames available in updateDisplayFrame" << std::endl;
      return;
    }
    
    // If we're still loading and have only the placeholder frame,
    // or if the requested frame is beyond what we've loaded,
    // show the first frame (which is our loading placeholder)
    int frameToShow = 0;
    if (!mIsLoading && mFrames.size() > 1 && mCurrentFrame < mFrames.size()) {
      frameToShow = mCurrentFrame;
    }
    
    // Make sure we have a valid texture before continuing
    if (!mVideo.videoTexture.created()) {
      std::cerr << "Texture not created in updateDisplayFrame" << std::endl;
      return;
    }
    
    // Make sure the frame is valid
    if (mFrames[frameToShow].empty()) {
      std::cerr << "Empty frame at index " << frameToShow << std::endl;
      return;
    }
    
    try {
      mVideo.videoImage = mFrames[frameToShow];
      mVideo.videoTexture.submit(mVideo.videoImage.ptr());
    } catch (const std::exception& e) {
      std::cerr << "Exception in updateDisplayFrame: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown exception in updateDisplayFrame" << std::endl;
    }
  }

  void resetRotation() {
    rotation.set(0.f);
    rotate.set(false);
  }

  VideoSphereLoaderCV() {
    mParams << mPlaying << mLooping << mRestarted << mCurrentTime << mVideoFilePath << networkedInitFlag;
    mParams << mPose << rotate << rotation << rotationSpeed;
    networkedInitFlag.registerChangeCallback([this](bool value) {
      this->initFlag = true;
      std::cout << "NetworkedInitFlag changed, setting initFlag to true" << std::endl;
    });
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
  
  bool loadVideo() {
    std::cout << "Loading video: " << mVideoFilePath.get() << std::endl;
    
    // Clean up any previous loading
    cleanup();
    
    // Create a sphere mesh for rendering
    addTexSphere(mMesh, 15.0, 24, true);
    
    // Initialize the video file
    mVideo.initializeVideoCaptureFile(mVideoFilePath, true);
    
    if (!mVideo.videoCapture || !mVideo.videoCapture->isOpened()) {
      std::cerr << "Error opening video file: " << mVideoFilePath.get() << std::endl;
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
    
    // Create placeholder frame
    cv::Mat loadingFrame(mVideoHeight, mVideoWidth, CV_8UC4, cv::Scalar(0, 0, 0, 255));
    try {
      cv::putText(loadingFrame, "Loading video...", 
                  cv::Point(mVideoWidth/2 - 100, mVideoHeight/2), 
                  cv::FONT_HERSHEY_SIMPLEX, 1.0, 
                  cv::Scalar(255, 255, 255, 255), 2);
    } catch (const cv::Exception& e) {
      std::cerr << "OpenCV exception creating loading frame: " << e.what() << std::endl;
    }
    
    // Initialize frames vector with placeholder
    {
      std::lock_guard<std::mutex> lock(mFramesMutex);
      mFrames.clear();
      mFrames.push_back(loadingFrame);
    }
    
    // Set the initial texture
    mVideo.videoImage = loadingFrame;
    mVideo.videoTexture.submit(mVideo.videoImage.ptr());
    
    // Start asynchronous loading
    if (mLoadingThread.joinable()) {
      mLoadingThread.join();
    }
    
    mIsLoading = true;
    mLoadingThread = std::thread([this]() {
      this->loadFramesAsync();
    });
    
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
    
    // If we're still loading, just show the loading frame
    if (mIsLoading) {
      return;
    }
    
    {
      std::lock_guard<std::mutex> lock(mFramesMutex);
      if (mFrames.empty() || mFrames.size() <= 1) {
        // Still loading or no frames available
        return;
      }
    }
    
    // Update current time
    mCurrentTime = mCurrentTime + (dt * speed);
    
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
      updateDisplayFrame();
    }
  }

  void onProcess(al::Graphics& g) {

    if (this->initFlag) {
      this->loadVideo();
      // this->resetRotation();
      this->initFlag = false;
    }

    if (!mVideo.videoTexture.created()) {
      std::cerr << "Texture not created in draw" << std::endl;
      return;
    }

    g.pushMatrix();

    if (rotate) {
      g.rotate(rotation.get(), 0, 1, 0);
      if (isPrimary()) {
        rotation = rotation + rotationSpeed; // Increment rotation
        if (rotation.get() > 360.f) {
          rotation.set(0.f); // Reset rotation after a full circle
        }        
      }
    }    

    mVideo.videoTexture.bind(0);
    g.texture();
    g.draw(mMesh);
    mVideo.videoTexture.unbind(0);
    g.popMatrix();
    if (!mIsReplica) {
      //mGUI.draw(g);
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
    // Stop the loading thread
    mIsLoading = false;
    if (mLoadingThread.joinable()) {
      mLoadingThread.join();
    }
    
    // Clear all preloaded frames to free memory
    {
      std::lock_guard<std::mutex> lock(mFramesMutex);
      mFrames.clear();
    }
    
    mVideo.cleanupVideoCapture();
    mPlaying = false;
  }
  
  // Asynchronous frame loading function
  void loadFramesAsync() {
    // Set to the beginning of the video
    mVideo.videoCapture->set(cv::CAP_PROP_POS_FRAMES, 0);
    
    // Create a temporary vector to hold the frames
    std::vector<cv::Mat> tempFrames;
    tempFrames.reserve(mFrameCount);
    
    // Read all frames
    cv::Mat frame;
    int frameCount = 0;
    
    try {
      while (frameCount < mFrameCount && mVideo.videoCapture->isOpened() && mIsLoading) {
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
        
        tempFrames.push_back(frameCopy);
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
    
    std::cout << "Loaded " << tempFrames.size() << " frames" << std::endl;
    
    // Replace the frames vector with our loaded frames
    {
      std::lock_guard<std::mutex> lock(mFramesMutex);
      mFrames = std::move(tempFrames);
    }
    
    mIsLoading = false;
    
    // We can now close the video file since we have all frames in memory
    mVideo.cleanupVideoCapture();
  }
};

#endif // EOYS_VIDEO_TO_SPHERE_CV
