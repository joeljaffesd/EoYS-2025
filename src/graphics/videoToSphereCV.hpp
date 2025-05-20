#ifndef EOYS_VIDEO_TO_SPHERE_CV
#define EOYS_VIDEO_TO_SPHERE_CV

#include "al/graphics/al_Shapes.hpp"
#include "al_ext/opencv/al_OpenCV.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include <vector>
#include <iostream>
#include <chrono>
#include <memory>
#include <algorithm> // For std::min

class VideoSphereLoaderCV : public al::PositionedVoice {
private:
  al::Mesh mMesh;
  al::AlloOpenCV mVideo;
  std::string mVideoFilePath;
  al::ControlGUI mGUI;
  
  // Frame cache - we'll keep a small buffer of recent frames
  static const int FRAME_CACHE_SIZE = 30; // Cache about 1 second of video
  std::vector<cv::Mat> mFrameCache;
  std::vector<int> mCachedFrameIndices; // To track which frames are in the cache
  
  // Video properties
  int mVideoWidth = 0;
  int mVideoHeight = 0;
  double aspectRatio = 1.0;
  double mFrameRate = 30.0;
  double mFrameTime = 1.0 / 30.0;
  int mFrameCount = 0;
  int mCurrentFrame = 0;
  int mLastRequestedFrame = -1;
  
  // Playback control
  al::ParameterBool mPlaying {"mPlaying", "", false};
  al::ParameterBool mLooping {"mLooping", "", true};
  al::ParameterBool mRestarted {"mRestarted", "", false};
  bool restartFlag = false; // hack for networked restart
  al::Parameter mCurrentTime {"mCurrentTime", "", 0, 0, std::numeric_limits<int>::max()};
  al::ParameterBundle mParams{"VideoSphereLoaderCV"};

public:
  // Get a frame from cache or load it from disk if needed
  cv::Mat getFrame(int frameIndex) {
    // Check if the requested frame is in cache
    for (size_t i = 0; i < mCachedFrameIndices.size(); i++) {
      if (mCachedFrameIndices[i] == frameIndex) {
        return mFrameCache[i];
      }
    }
    
    // Frame not in cache, need to load it from disk
    if (!mVideo.videoCapture || !mVideo.videoCapture->isOpened()) {
      std::cerr << "Video capture not open in getFrame" << std::endl;
      return cv::Mat(); // Return empty frame
    }
    
    // Seek to the requested frame
    mVideo.videoCapture->set(cv::CAP_PROP_POS_FRAMES, frameIndex);
    
    // Read the frame
    cv::Mat frame;
    bool success = mVideo.videoCapture->read(frame);
    if (!success || frame.empty()) {
      std::cerr << "Failed to read frame " << frameIndex << std::endl;
      return cv::Mat();
    }
    
    // Add to cache (using simple FIFO)
    if (mFrameCache.size() >= FRAME_CACHE_SIZE) {
      // Remove oldest frame
      mFrameCache.erase(mFrameCache.begin());
      mCachedFrameIndices.erase(mCachedFrameIndices.begin());
    }
    
    // Add new frame to cache
    mFrameCache.push_back(frame.clone());
    mCachedFrameIndices.push_back(frameIndex);
    
    return frame;
  }
  
  // Update the displayed frame based on the current frame index
  void updateDisplayFrame() {
    if (mCurrentFrame < 0 || mCurrentFrame >= mFrameCount) {
      std::cerr << "Invalid frame index: " << mCurrentFrame << std::endl;
      return;
    }
    
    // Get the frame (from cache or disk)
    cv::Mat frame = getFrame(mCurrentFrame);
    
    if (frame.empty()) {
      std::cerr << "Empty frame at index " << mCurrentFrame << std::endl;
      return;
    }
    
    try {
      // Make sure we have a valid texture
      if (!mVideo.videoTexture.created()) {
        mVideo.videoTexture.create2D(frame.cols, frame.rows);
        mVideo.videoTexture.filter(al::Texture::LINEAR);
        mVideo.videoTexture.wrap(al::Texture::REPEAT, al::Texture::CLAMP_TO_EDGE, al::Texture::CLAMP_TO_EDGE);
      }
      
      mVideo.videoImage = frame;
      mVideo.videoTexture.submit(mVideo.videoImage.ptr());
      mLastRequestedFrame = mCurrentFrame;
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
    // Make sure to clean up resources when this object is destroyed
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
    this->loadVideo("../assets/videos/vid.mp4");

    if (ImGui::GetCurrentContext() == nullptr) {
      al::imguiInit();
    }
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
    
    // Initialize the frame cache
    mFrameCache.clear();
    mCachedFrameIndices.clear();
    
    // Pre-cache first few frames for smoother start
    for (int i = 0; i < std::min(10, mFrameCount); i++) {
      cv::Mat frame = getFrame(i);
      if (frame.empty()) {
        std::cerr << "Warning: Failed to pre-cache frame " << i << std::endl;
      }
    }
    
    // Set the first frame
    mCurrentFrame = 0;
    mLastRequestedFrame = -1;
    updateDisplayFrame();
    
    // Start playback
    mPlaying = true;
    
    return true;
  }
  
  void update(double dt) override {
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
      // For large jumps, clear the cache to avoid filling it with unnecessary frames
      if (std::abs(targetFrame - mCurrentFrame) > FRAME_CACHE_SIZE) {
        mFrameCache.clear();
        mCachedFrameIndices.clear();
      }
      
      mCurrentFrame = targetFrame;
      updateDisplayFrame();
    }
    
    // Predict and pre-cache upcoming frames for smoother playback
    if (mPlaying && targetFrame + 1 < mFrameCount && targetFrame + 1 != mLastRequestedFrame) {
      // Asynchronous pre-caching could be implemented here for even better performance
      getFrame(targetFrame + 1);
    }
  }

  void onProcess(al::Graphics& g) {
    
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
    // Clear the frame cache
    mFrameCache.clear();
    mCachedFrameIndices.clear();
    
    // Close the video file
    mVideo.cleanupVideoCapture();
    mPlaying = false;
  }
};

#endif // EOYS_VIDEO_TO_SPHERE_CV
