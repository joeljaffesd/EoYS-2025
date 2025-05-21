#ifndef EOYS_VIDEO_TO_SPHERE
#define EOYS_VIDEO_TO_SPHERE

#include "al/graphics/al_Shapes.hpp"
#include "al_ext/video/al_VideoDecoder.hpp"
#include <vector> // Added for std::vector
#include <iostream>
#include <chrono>

class VideoSphereLoader {
private:
  Mesh mMesh;
  Texture tex;
  VideoDecoder videoDecoder;
  std::string mVideoFileToLoad;
  
  // RGB Buffer for frame conversion
  std::vector<uint8_t> mRgbaBuffer;
  
  // Video properties
  int mVideoWidth = 0;
  int mVideoHeight = 0;
  double aspectRatio = 1.0;
  
  // Playback control
  al::ParameterBool mPlaying {"mPlaying", "", false};
  al::ParameterBool mLooping {"mLooping", "", false};
  al::ParameterBool mRestarted {"mRestart", "", false};
  al::ParameterInt mFrameNumber {"mFrameNumber", "", 0, 0, std::numeric_limits<int>::max()};
  al::ParameterBundle mParams{"VideoSphereLoader"};

public:
  VideoSphereLoader()  {
    mParams << mPlaying << mLooping << mRestarted << mFrameNumber;
    
    // Register a callback for the restart parameter
    mRestarted.registerChangeCallback([this](bool value) {
        videoDecoder.seek(0);  // Seek to the beginning of the video
    });
  }

  ~VideoSphereLoader() {
    this->cleanup();
  }

  al::ParameterBundle& params() {
    return this->mParams;
  }
  
  bool loadVideo(const std::string& videoFilePath) {
    addTexSphere(mMesh, 15.0, 24, true);
    mVideoFileToLoad = videoFilePath;
    
    // Load the video file
    videoDecoder.enableAudio(false);
    if (!videoDecoder.load(mVideoFileToLoad.c_str())) {
      std::cerr << "Error loading video file" << std::endl;
      return false;
    }
    
    // Set up texture configuration
    tex.filter(Texture::LINEAR);
    tex.wrap(Texture::REPEAT, Texture::CLAMP_TO_EDGE, Texture::CLAMP_TO_EDGE);
    tex.create2D(videoDecoder.width(), videoDecoder.height(), Texture::RGBA8,
                 Texture::RGBA, Texture::UBYTE);

    // Set video playback parameters
    mVideoWidth = videoDecoder.width();
    mVideoHeight = videoDecoder.height();
    aspectRatio = mVideoWidth / (double)mVideoHeight;
    
    // Allocate an RGBA buffer for the converted frame
    mRgbaBuffer.resize(mVideoWidth * mVideoHeight * 4);
    
    // Start the video decoder thread
    videoDecoder.start();
    
    // Start playback
    mPlaying = true;
    
    return true;
  }
  
  void update(double dt) {
    videoDecoder.pause(!mPlaying);
    videoDecoder.loop(mLooping);

    // Get a new frame from the video decoder
    MediaFrame* frame = videoDecoder.getVideoFrame(dt);
    
    if (frame) {
      // Convert YUV to RGBA
      convertYUVToRGBA(frame, mRgbaBuffer.data(), mVideoWidth, mVideoHeight);
      
      // Submit the converted RGBA data to the texture
      tex.submit(mRgbaBuffer.data());
      
      // Tell the decoder we're done with this frame
      videoDecoder.gotVideoFrame();
      
    }
  }

  void draw(Graphics& g) {
    g.pushMatrix();
    tex.bind(0);
    g.texture();
    g.draw(mMesh);
    tex.unbind(0);
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

  // Toggle looping FIXME
  void toggleLooping() { mLooping = !mLooping; }
  
  // Getters for video properties
  double getAspectRatio() const { return aspectRatio; }
  
  void cleanup() {
    videoDecoder.stop();
    videoDecoder.cleanup();
  }
  
  // YUV to RGB conversion function
  void convertYUVToRGBA(MediaFrame* frame, uint8_t* rgbaData, int width, int height) {
    // Basic YUV to RGB conversion (BT.601 standard)
    // This is a simple conversion - more sophisticated conversion would use proper color matrices
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        int index = i * width + j;
        int rgbaIndex = index * 4;
        
        int y = frame->dataY[index];
        
        // If we have U and V data (some formats might only have Y)
        int u = 128;  // Default to middle value if no U component
        int v = 128;  // Default to middle value if no V component
        
        if (!frame->dataU.empty() && !frame->dataV.empty()) {
          // U and V might be at lower resolution (usually half)
          int uvIndex = (i / 2) * (width / 2) + (j / 2);
          if (uvIndex < frame->dataU.size()) {
            u = frame->dataU[uvIndex];
            v = frame->dataV[uvIndex];
          }
        }
        
        // YUV to RGB conversion
        int r = y + 1.402 * (v - 128);
        int g = y - 0.344 * (u - 128) - 0.714 * (v - 128);
        int b = y + 1.772 * (u - 128);
        
        // Clamp values to 0-255 range
        r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
        g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
        b = (b < 0) ? 0 : ((b > 255) ? 255 : b);
        
        // Store RGBA values
        rgbaData[rgbaIndex + 0] = r;     // R
        rgbaData[rgbaIndex + 1] = g;     // G
        rgbaData[rgbaIndex + 2] = b;     // B
        rgbaData[rgbaIndex + 3] = 255;   // A (fully opaque)
      }
    }
  }
};

#endif // EOYS_VIDEO_TO_SPHERE 