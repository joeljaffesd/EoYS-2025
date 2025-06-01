// file for testing loading in shaders onto sphere

#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/shaderEngine.hpp"
#include "src/audio/audioManager.hpp"

struct MyApp : al::DistributedApp {
  al::DistributedScene mScene;

  void onInit() override {
    al::imguiInit();
    mScene.prepare(this->audioIO());
    mScene.verbose(true);
    mScene.registerSynthClass<ShaderEngine>();
    registerDynamicScene(mScene);
  }

  void onSound(al::AudioIOData &io) override {
    if (isPrimary()) {
      mScene.render(io);
    }
  }

  int activeVoiceId = -1;
  bool onKeyDown(const al::Keyboard& k) override {
    if (isPrimary() && k.key() == ' ') {
      mScene.triggerOff(activeVoiceId);
      auto* voice = mScene.getVoice<ShaderEngine>();
      voice->shaderPath("../src/shaders/SunExplode.frag");
      activeVoiceId = mScene.triggerOn(voice);
    }
    return true;
  }

  void onAnimate(double dt) override {
    mScene.update(dt);
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0);
    mScene.render(g);
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 128, 2, 1);
  app.start();
}