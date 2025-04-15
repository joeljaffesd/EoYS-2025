#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "graphics/imageToSphere.hpp"

using namespace al;

class ImageTestApp : public App {
public:
  ImageSphereLoader imageSphereLoader;

  void onCreate() override {
    // Initialize the image sphere loader
    imageSphereLoader.init();
    imageSphereLoader.createSphere();
  }

  void onDraw(Graphics &g) override {
    g.clear(0); // Clear the screen
    imageSphereLoader.draw(g); // Draw the sphere
  }
};

int main() {
  ImageTestApp app;
  app.start();
  return 0;
}