#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/cameraToSphere.hpp"
#include "al/ui/al_ControlGUI.hpp"
using namespace al;

class CameraApp : public DistributedApp {
public:
  CameraSphereLoader loader;
  ParameterBundle mBundle;
  ControlGUI mGUI;
  
  void onInit() override {
    mGUI.init();
    this->parameterServer().registerParameterBundle(loader.params());
    mGUI.registerParameterBundle(loader.params());
  }
  
  void onCreate() override {
    // Initialize the camera
    loader.initCamera(0);
  }

  void onAnimate(double dt) override {
    // Update the camera sphere loader with the elapsed time
    loader.update(dt);
  }

  void onDraw(Graphics &g) override {
    g.clear(0.1f);
    loader.draw(g);
    if (isPrimary()) {
      mGUI.draw(g);
    }
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {  // Spacebar toggles camera on/off
      loader.toggleCamera();
    } else if (k.key() == 'r') {  // 'r' to reset camera view
      nav().home();
    } else if (k.key() == '1' || k.key() == '2' || k.key() == '3') {  // Number keys to switch cameras
      // Switch to camera 0, 1, or 2
      int deviceIndex = k.key() - '1';
      loader.setDevice(deviceIndex);
    }
    return true;
  }
};

int main() {
  CameraApp app;
  app.configureAudio(0, 0, 0, 0); // Disable audio
  app.start();
  return 0;
}
