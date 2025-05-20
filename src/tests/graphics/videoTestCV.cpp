#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/videoToSphereCV.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include <iostream>
#include <vector>
#include <unistd.h> // For getcwd

class VideoAppCV : public al::DistributedApp {
public:
  VideoSphereLoaderCV* mVideoSphereLoaderCV;
  al::DistributedScene mDistributedScene;
  al::ParameterBundle mBundle;

  void onInit() override {
    al::imguiInit();

    mDistributedScene.verbose(true);
    mDistributedScene.registerSynthClass<VideoSphereLoaderCV>();
    this->registerDynamicScene(mDistributedScene);
    mVideoSphereLoaderCV = mDistributedScene.getVoice<VideoSphereLoaderCV>();
    mVideoSphereLoaderCV->loadVideo("../assets/videos/sky.mp4");
    mDistributedScene.triggerOn(mVideoSphereLoaderCV);
    
    // Setup camera for 3D viewing
    nav().pos(0, 0, 0);  // Position the camera
    nav().faceToward(al::Vec3f(0, 0, 0));  // Face toward origin
  }

  void onAnimate(double dt) override {
    // Update the video sphere loader with the elapsed time
    mDistributedScene.update(dt);
  }

  bool onKeyDown(const al::Keyboard& k) override {
    if (k.key() == ' ') {  // Spacebar toggles play/pause
      mVideoSphereLoaderCV->togglePlayPause();
    } else if (k.key() == 'r') {  // 'r' to restart
      mVideoSphereLoaderCV->restart();
    } else if (k.key() == 'l') {  // 'l' to toggle looping
      mVideoSphereLoaderCV->toggleLooping();
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
