#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "../../graphics/imageToSphere.hpp"
#include "../../graphics/vfxMain.hpp"
#include "../../graphics/autoPulse.hpp"
#include "../../graphics/orbit.hpp"
#include "../../graphics/vfxUtility.hpp"

using namespace al;

class ImageTestApp : public App {
public:
  ImageSphereLoader imageSphereLoader;
  RippleEffect rippleZ;
  RippleEffect rippleY;
  AutoPulseEffect pulse;
  OrbitEffect orbit;
  VertexEffectChain vEffectChain;
  VertexEffectChain vEffectChain2;
  VAOMesh sphereMesh;

  void onCreate() override {
    // Initialize the image sphere loader
    imageSphereLoader.init();
    imageSphereLoader.createSphere();
    sphereMesh.primitive(Mesh::POINTS);

    pulse.setBaseMesh(imageSphereLoader.mMesh.vertices());

    // effects on image mesh
    rippleZ.setParams(1.0, 0.5, 4.0, 'z');
    rippleY.setParams(2.0, 0.1, 6.0, 'y');
    pulse.setParams(0.5, 0.1, 1);

    vEffectChain.pushBack(&pulse);
    vEffectChain.pushBack(&rippleZ);
    vEffectChain.pushBack(&rippleY);
    // end effects on image mesh

    // effect on test ball
    orbit.setParams(0.7, 1.0, al::Vec3f{0, 0, 0}, 1);
    vEffectChain2.pushBack(&orbit);

    // for seeing perspective
    addSphere(sphereMesh, 0.2, 16, 16);
    sphereMesh.colorFill({1, 1, 1});
    // sphereMesh.translate(0.2, 0.2, 0.2);
    sphereMesh.update();
  }
  double t;
  void onAnimate(double dt) override {
    t += dt;
    vEffectChain.process(imageSphereLoader.mMesh, t);
    vEffectChain2.process(sphereMesh, t);
  }

  void onDraw(Graphics &g) override {
    g.clear(0); // Clear the screen
    g.meshColor();
    g.pointSize(imageSphereLoader.pointSize); // play with
    g.draw(imageSphereLoader.mMesh);
    g.draw(sphereMesh);
    // imageSphereLoader.draw(g); // Draw the sphere - old way
  }
};

int main() {
  ImageTestApp app;
  app.start();
  return 0;
}