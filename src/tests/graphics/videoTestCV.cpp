#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/videoToSphereCV.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include <iostream>
#include <vector>
#include <unistd.h> // For getcwd
using namespace al;

class VideoAppCV : public DistributedApp {
public:
  VideoSphereLoaderCV loader;
  ParameterBundle mBundle;
  ControlGUI mGUI;

  void onInit() override {
    mGUI.init();

    // breaks shit rn
    this->parameterServer().registerParameterBundle(loader.params());
    mGUI.registerParameterBundle(loader.params());

    // Print working directory for debugging
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      std::cout << "Current working directory: " << cwd << std::endl;
    }
    
    // Try different paths to find the video file
    std::vector<std::string> paths = {
      "../assets/videos/vid.mp4",
      "./assets/videos/vid.mp4",
      "../../assets/videos/vid.mp4",
      "/Users/joeljaffesd/Desktop/temp/EoYS-2025/assets/videos/vid.mp4"
    };
    
    bool loaded = false;
    for (const auto& path : paths) {
      std::cout << "Trying to load video from: " << path << std::endl;
      if (loader.loadVideo(path)) {
        std::cout << "Successfully loaded video from: " << path << std::endl;
        loaded = true;
        break;
      }
    }
    
    if (!loaded) {
      std::cerr << "Failed to load video from any path!" << std::endl;
    }
    
    // Setup camera for 3D viewing
    nav().pos(0, 0, 20);  // Position the camera
    nav().faceToward(Vec3f(0, 0, 0));  // Face toward origin
  }

  void onAnimate(double dt) override {
    // Update the video sphere loader with the elapsed time
    loader.update(dt);
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {  // Spacebar toggles play/pause
      loader.togglePlayPause();
    } else if (k.key() == 'r') {  // 'r' to restart
      loader.restart();
    } else if (k.key() == 'l') {  // 'l' to toggle looping
      loader.toggleLooping();
    } 
    return true;
  }

  void onDraw(Graphics &g) override {
    g.clear(0.1f);
    loader.draw(g);
    if (isPrimary()) { mGUI.draw(g); }
  }
};

int main() {
  VideoAppCV app;
  app.configureAudio(0, 0, 0, 0); // Disable audio
  app.start();
  return 0;
}
