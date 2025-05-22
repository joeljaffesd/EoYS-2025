#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "../../graphics/imageToSphere.hpp"
#include "../../graphics/vfxMain.hpp"
#include "../../graphics/autoPulse.hpp"
#include "../../graphics/orbit.hpp"
#include "../../graphics/vfxUtility.hpp"

#include "al/scene/al_DistributedScene.hpp"

using namespace al;

class ImageTestApp : public DistributedApp {
public:
  al::DistributedScene mDistributedScene;
  ImageSphereLoader* mImageSphereLoader;
  RippleEffect rippleZ;
  RippleEffect rippleY;
  AutoPulseEffect pulse;
  OrbitEffect orbit;
  VertexEffectChain vEffectChain;
  VertexEffectChain vEffectChain2;

  void onInit() override {
    al::imguiInit();
    mDistributedScene.verbose(true);
    mDistributedScene.registerSynthClass<ImageSphereLoader>();
    registerDynamicScene(mDistributedScene);
  }

  void onCreate() override {

    // better if this function took the mesh itself as the arg
    if (mImageSphereLoader != nullptr) {
      pulse.setBaseMesh(mImageSphereLoader->mMesh.vertices());
    }
    
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
  }

  bool onKeyDown(const Keyboard& k) override {
    if (isPrimary() && k.key() == ' ') {
      if (mImageSphereLoader == nullptr) {
        mImageSphereLoader = mDistributedScene.getVoice<ImageSphereLoader>();
        mDistributedScene.triggerOn(mImageSphereLoader);
      }
    }
  }

  double t;
  bool initFlag = true;
  void onAnimate(double dt) override {
    if (mImageSphereLoader != nullptr && initFlag) {
      pulse.setBaseMesh(mImageSphereLoader->mMesh.vertices());
      initFlag = false;
    }

    t += dt;
    if (mImageSphereLoader != nullptr) {
      vEffectChain.process(mImageSphereLoader->mMesh, t);
    }
    mDistributedScene.update(dt);
  }

  void onDraw(Graphics &g) override {
    g.clear(0); // Clear the screen
    g.meshColor();
    mDistributedScene.render(g);
  }
};

int main() {
  ImageTestApp app;
  app.start();
  return 0;
}