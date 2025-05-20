// file for testing loading in shaders onto sphere

#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/shaderEngine.hpp"
#include "src/audio/audioManager.hpp"

struct MyApp : al::DistributedApp {
  DistributedSceneWithInput mScene;

  void onInit() override {

    mScene.setVoiceMaxInputChannels(8);
    mScene.prepare(this->audioIO());

    mScene.verbose(true);
    mScene.registerSynthClass<ShaderEngine>();
    registerDynamicScene(mScene);
    auto* bruh = mScene.getVoice<ShaderEngine>();
    mScene.triggerOn(bruh);
  }

  void onSound(al::AudioIOData &io) override {
    if (isPrimary()) {
      mScene.render(io);
    }
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