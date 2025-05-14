#include "al/app/al_DistributedApp.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "../../graphics/assetEngine.hpp"

class AssetTestApp : public al::DistributedApp {
public:
  AssetEngine assetEngine;
  al::ControlGUI gui;
  al::ParameterBool rotate{"Rotate", "", true}; // Toggle rotation

  void onCreate() override {
    // Load the 3D asset
    assetEngine.loadAsset("../assets/3dModels/eye/eye.obj",
                          "../assets/3dModels/eye/eye.png");

    // Initialize GUI
    gui.init();
    gui << rotate; // Add rotation toggle to GUI
  }

  void onAnimate(double dt) override {
    // Enable or disable rotation based on the GUI toggle
    if (!rotate.get()) {
      assetEngine.a = 0.0f; // Stop rotation
    }
  }

  void onDraw(al::Graphics &g) override {
    g.clear(0.1); // Clear the screen with a dark gray background
    assetEngine.draw(g); // Draw the 3D object
    gui.draw(g); // Draw the GUI
  }

  void onExit() override {
    gui.cleanup();
  }
};

int main() {
  AssetTestApp app;
  app.start();
  return 0;
}