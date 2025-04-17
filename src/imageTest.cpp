#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "graphics/imageToSphere.hpp"
#include "graphics/vfxMain.hpp"
#include "src/graphics/autoPulse.hpp"
#include "src/graphics/vfxUtility.hpp"

using namespace al;

class ImageTestApp : public App {
public:
  ImageSphereLoader imageSphereLoader;
  RippleEffect rippleZ;
  RippleEffect rippleY;
  AutoPulseEffect pulse;
  VertexEffectChain vEffectChain;

  void onCreate() override {
    // Initialize the image sphere loader
    imageSphereLoader.init();
    imageSphereLoader.createSphere();

    pulse.setBaseMesh(imageSphereLoader.mMesh.vertices());

    rippleZ.setParams(1.0, 0.5, 4.0, 'z');
    rippleY.setParams(2.0, 0.1, 6.0, 'y');
    pulse.setParams(0.5, 0.1, 1);

    vEffectChain.pushBack(&pulse);
    vEffectChain.pushBack(&rippleZ);
    vEffectChain.pushBack(&rippleY);
  }
  double t;
  void onAnimate(double dt) override {
    t += dt;
    vEffectChain.process(imageSphereLoader.mMesh, t);
  }

  void onDraw(Graphics &g) override {
    g.clear(0); // Clear the screen
    g.meshColor();
    g.pointSize(imageSphereLoader.pointSize); // play with
    g.draw(imageSphereLoader.mMesh);
    // imageSphereLoader.draw(g); // Draw the sphere - old way
  }
};

int main() {
  ImageTestApp app;
  app.start();
  return 0;
}