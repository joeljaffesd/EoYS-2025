#ifndef EOYS_GRAPHICS_HPP
#define EOYS_GRAPHICS_HPP

#include "imageToSphere.hpp"
#include "assetEngine.hpp"
#include "shaderEngine.hpp"
#include "videoToSphereCV.hpp"

/**
 * @brief TODO
 */
class GraphicsManager {
private:
  al::ParameterInt index { "Index", "", 0, 0, std::numeric_limits<int>::max() };
  al::ParameterInt type { "Type", "", 0, 0, 3 };
  al::ParameterBool updateFlag { "updateFlag", "", false };
  bool localUpdateFlag = false;
  std::vector<std::function<void()>> mCallbacks;
  ImageSphereLoader mImageLoader; // works 
  AssetEngine mAssetEngine; // halfway works
  ShaderEngine mShaderEngine; // halfway works 
  VideoSphereLoaderCV mVideoLoader; // halfway works

public:
  GraphicsManager() {
    // callback on index change
    this->index.registerChangeCallback([this](int value){ 
      std::cout << "Index at: " << value << std::endl;
    });

    mCallbacks.push_back([this]() {
      mImageLoader.loadImage("../assets/images/imgWrap.png");
      type = 0;
    });

    mCallbacks.push_back([this]() {
      mAssetEngine.loadAsset("../assets/3dModels/eye/eye.obj",
                             "../assets/3dModels/eye/eye.png");
      type = 1;
    });

    mCallbacks.push_back([this]() {
      mShaderEngine.shader("../src/shaders/Reactive-shaders/fractal1.frag");
      type = 2;
    });

    mCallbacks.push_back([this]() {
      mVideoLoader.loadVideo("../assets/videos/vid.mp4");
      type = 3;
    });

    mCallbacks.push_back([this]() {
      mVideoLoader.loadVideo("../assets/scenes/manInTheBox/inTheBox.mp4");
      type = 3;
    });

    mCallbacks.push_back([this]() {
      mVideoLoader.loadVideo("../assets/scenes/manInTheBox/homunculusBG.mp4");
      type = 3;
    });

  }

  ~GraphicsManager() {}

  void registerParameters(al::ParameterServer& server) {
    server.registerParameter(index);
    server.registerParameter(type);
    server.registerParameter(updateFlag);
    // TODO 
    server.registerParameterBundle(mShaderEngine.params());
    server.registerParameterBundle(mImageLoader.params());
    server.registerParameterBundle(mAssetEngine.params());
    server.registerParameterBundle(mVideoLoader.params());
  }

  /**
   * @brief TODO
   */
  void init(bool isReplica = false) {
    if (isReplica) {
      mImageLoader.init(true);
      mAssetEngine.init(true);
      mShaderEngine.init(true);
      mVideoLoader.init(true);
    } else {
      mImageLoader.init();
      mAssetEngine.init();
      mShaderEngine.init();
      mVideoLoader.init();
    }
  }

  /**
   * @brief TODO
   */
  void update(double dt = 0) {
    if (index >= mCallbacks.size()) {
      this->prevScene();
    }

    if (updateFlag != localUpdateFlag) {
      localUpdateFlag = !localUpdateFlag;
      mCallbacks[index]();
    }

    mImageLoader.update(dt);
    mAssetEngine.update(dt);
    mShaderEngine.update(dt);
    mVideoLoader.update(dt);
  }

  /**
   * @brief TODO
   */
  void render(al::Graphics& g) {
    switch (type) {
      case 0:
        mImageLoader.onProcess(g);
        break;
      case 1:
        mAssetEngine.onProcess(g);
        break;
      case 2:
        mShaderEngine.onProcess(g);
        break;
      case 3:
        mVideoLoader.onProcess(g);
        break;
      default:
        std::cout << "Invalid Type" << std::endl;
    }
  }

  /**
   * @brief TODO
   */
  void render(al::AudioIOData& io) {
    mShaderEngine.onProcess(io);
  }

  /**
   * @brief TODO
   */
  void nextScene() {
    this->index = index + 1;
    updateFlag = !updateFlag;
  } 

  /**
   * @brief TODO
   */
  void prevScene() {
    this->index = index - 1;
    updateFlag = !updateFlag;
  } 

};

#endif