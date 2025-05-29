#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/videoToSphereCV.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include <iostream>
#include <vector>
#include <unistd.h> // For getcwd

class VideoAppCV : public al::DistributedApp {
public:
  VideoSphereLoaderCV* mVideoSphereLoaderCV = nullptr;
  al::DistributedScene mDistributedScene;

  void onInit() override {
    al::imguiInit();
    mDistributedScene.verbose(true);
    mDistributedScene.registerSynthClass<VideoSphereLoaderCV>();
    this->registerDynamicScene(mDistributedScene);
  }

  void onAnimate(double dt) override {
    mDistributedScene.update(dt);
  }

  bool onKeyDown(const al::Keyboard& k) override {
    if (isPrimary()) {
      if (mVideoSphereLoaderCV == nullptr) {
        if (k.key() == ' ') {
          mVideoSphereLoaderCV = mDistributedScene.getVoice<VideoSphereLoaderCV>();
          mDistributedScene.triggerOn(mVideoSphereLoaderCV);
        }
      } else {
        if (k.key() == ' ') {
          mVideoSphereLoaderCV->togglePlayPause();
        } else if (k.key() == 'r') {  // 'r' to restart
          mVideoSphereLoaderCV->restart();
        } else if (k.key() == 'l') {  // 'l' to toggle looping
          mVideoSphereLoaderCV->toggleLooping();
        }
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
  VideoAppCV app;
  app.configureAudio(0, 0, 0, 0); // Disable audio
  app.start();
  return 0;
}
