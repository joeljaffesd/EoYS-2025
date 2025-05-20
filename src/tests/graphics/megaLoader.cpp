#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/videoToSphereCV.hpp"
#include "../../graphics/imageToSphere.hpp"
#include "../../graphics/assetEngine.hpp"
#include "../../graphics/shaderEngine.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "../../audio/audioManager.hpp"

class MegaLoader : public al::DistributedApp {
public:
  al::PositionedVoice* activeVoice;
  int activeVoiceId;
  DistributedSceneWithInput mDistributedScene;
  al::ParameterBundle mBundle;

  void onInit() override {

    mDistributedScene.setVoiceMaxInputChannels(8);

    mDistributedScene.verbose(true);
    mDistributedScene.registerSynthClass<ImageSphereLoader>();
    mDistributedScene.registerSynthClass<AssetEngine>();
    mDistributedScene.registerSynthClass<ShaderEngine>();
    mDistributedScene.registerSynthClass<VideoSphereLoaderCV>();
    this->registerDynamicScene(mDistributedScene);
    
    // Setup camera for 3D viewing
    nav().pos(0, 0, 0);  // Position the camera
    nav().faceToward(al::Vec3f(0, 0, 0));  // Face toward origin
  }

  void onSound(al::AudioIOData& io) override {
    mDistributedScene.render(io);
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
          activeVoice = mDistributedScene.getVoice<ShaderEngine>();
          activeVoiceId = mDistributedScene.triggerOn(activeVoice);
          phase++;
          break;
        }
        case 3: {
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
  app.configureAudio(48000, 128, 2, 1);
  app.start();
  return 0;
}
