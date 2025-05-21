#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/videoToSphere.hpp"
#include "al/ui/al_ControlGUI.hpp"

class VideoApp : public DistributedApp {
public:
  VideoSphereLoader loader;
  ParameterBundle mBundle;
  ControlGUI mGUI;

  void onInit() override {
    mGUI.init();
    this->parameterServer().registerParameterBundle(loader.params());
    mGUI.registerParameterBundle(loader.params());
  }

  virtual void onCreate() override {
    // Set the video file path
    loader.loadVideo("../assets/videos/vid.mp4");
  }

  virtual void onAnimate(double dt) override {
    // Update the video sphere loader with the elapsed time
    loader.update(dt);
  }

  virtual bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {  // Spacebar toggles play/pause
      loader.togglePlayPause();
    } else if (k.key() == 'r') {  // 'r' to restart
      loader.restart();
    } else if (k.key() == 'l') {  // 'l' to toggle looping
      loader.toggleLooping();
    }
    return true;
  }

  virtual void onDraw(Graphics &g) override {
    g.clear(0.1f);
    loader.draw(g);
    if (isPrimary()) {
      mGUI.draw(g);
    }
  }

};

int main() {
  VideoApp app;
  app.start();
  return 0;
}