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

  }

  ~GraphicsManager() {}

  void registerParameters(al::ParameterServer& server) {
    server.registerParameter(index);
    // TODO 
    server.registerParameterBundle(mShaderEngine.params());
    server.registerParameterBundle(mImageLoader.params());
    server.registerParameterBundle(mAssetEngine.params());
    server.registerParameterBundle(mVideoLoader.params());
  }

  /**
   * @brief TODO
   */
  void init() {
    mImageLoader.init();
    mAssetEngine.init();
    mShaderEngine.init();
    mVideoLoader.init();
  }

  /**
   * @brief TODO
   */
  void update(double dt = 0) {
    mImageLoader.update(dt);
    mAssetEngine.update(dt);
    mShaderEngine.update(dt);
    mVideoLoader.update(dt);
  }

  /**
   * @brief TODO
   */
  void render(al::Graphics& g) {
    switch (index % 4) {
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
        std::cout << "Invalid index" << std::endl;
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
  } 

  /**
   * @brief TODO
   */
  void prevScene() {
    this->index = index - 1;
  } 


};

#endif