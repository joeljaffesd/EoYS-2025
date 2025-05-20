#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/videoToSphereCV.hpp"
#include "../../graphics/imageToSphere.hpp"
#include "../../graphics/assetEngine.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include <iostream>
#include <vector>
#include <unistd.h> // For getcwd

class MegaLoader : public al::DistributedApp {
public:
  al::PositionedVoice* activeVoice;
  int activeVoiceId;
  al::DistributedScene mDistributedScene;
  al::ParameterBundle mBundle;

  void onInit() override {
    al::imguiInit();

    mDistributedScene.verbose(true);
    mDistributedScene.registerSynthClass<VideoSphereLoaderCV>();
    mDistributedScene.registerSynthClass<ImageSphereLoader>();
    mDistributedScene.registerSynthClass<AssetEngine>();
    this->registerDynamicScene(mDistributedScene);
    
    // Setup camera for 3D viewing
    nav().pos(0, 0, 0);  // Position the camera
    nav().faceToward(al::Vec3f(0, 0, 0));  // Face toward origin
  }

  void onAnimate(double dt) override {
    // Update the video sphere loader with the elapsed time
    mDistributedScene.update(dt);
  }

  int phase = 0;
  bool onKeyDown(const al::Keyboard& k) override {
    if (k.key() == ' ') {
      mDistributedScene.triggerOff(activeVoiceId);
      switch (phase) {
        case 0: {
          activeVoice = mDistributedScene.getVoice<ImageSphereLoader>();
          activeVoiceId = mDistributedScene.triggerOn(activeVoice);
          phase++;
          break;
        }
        case 1: {
          activeVoice = mDistributedScene.getVoice<AssetEngine>();
          activeVoiceId = mDistributedScene.triggerOn(activeVoice);
          phase++;
          break;
        }
        case 2: {
          activeVoice = mDistributedScene.getVoice<VideoSphereLoaderCV>();
          activeVoiceId = mDistributedScene.triggerOn(activeVoice);
          phase++;
          break;
        }
        default:
          phase = 0;
          break;
      }
    }
    return true;
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0.1f);
    mDistributedScene.render(g);
  }
};

int main() {
  MegaLoader app;
  app.configureAudio(0, 0, 0, 0); // Disable audio
  app.start();
  return 0;
}
