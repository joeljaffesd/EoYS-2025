#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/videoToSphereCV.hpp"
#include "../../graphics/imageToSphere.hpp"
#include "../../graphics/assetEngine.hpp"
#include "../../graphics/shaderEngine.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "../../audio/audioManager.hpp"

class MegaLoader : public al::DistributedApp {
public:
  int activeVoiceId;
  DistributedSceneWithInput mDistributedScene;
  al::ParameterBundle mBundle;
  std::vector<std::function<void()>> loaderCalbacks;

  void onInit() override {

    loaderCalbacks.push_back([this]() {
      auto* voice = mDistributedScene.getVoice<ImageSphereLoader>();
      activeVoiceId = mDistributedScene.triggerOn(voice);
    });

    loaderCalbacks.push_back([this]() {
      auto* voice = mDistributedScene.getVoice<AssetEngine>();
      activeVoiceId = mDistributedScene.triggerOn(voice);
    });

    loaderCalbacks.push_back([this]() {
      auto* voice = mDistributedScene.getVoice<ShaderEngine>();
      activeVoiceId = mDistributedScene.triggerOn(voice);
    });
    
    loaderCalbacks.push_back([this]() {
      auto* voice = mDistributedScene.getVoice<VideoSphereLoaderCV>();
      activeVoiceId = mDistributedScene.triggerOn(voice);
    });

    mDistributedScene.setVoiceMaxInputChannels(8);

    mDistributedScene.verbose(true);
    mDistributedScene.registerSynthClass<ImageSphereLoader>();
    mDistributedScene.registerSynthClass<AssetEngine>();
    mDistributedScene.registerSynthClass<ShaderEngine>();
    mDistributedScene.registerSynthClass<VideoSphereLoaderCV>();
    this->registerDynamicScene(mDistributedScene);
  }

  void onSound(al::AudioIOData& io) override {
    mDistributedScene.render(io);
  }

  void onAnimate(double dt) override {
    mDistributedScene.update(dt);
  }

  int phase = 0;
  bool onKeyDown(const al::Keyboard& k) override {
    if (k.key() == ' ') {
      mDistributedScene.triggerOff(activeVoiceId);
      if (phase < loaderCalbacks.size()) {
        loaderCalbacks[phase]();
        phase++;
      } else {
        phase = 0;
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