#include "al/app/al_DistributedApp.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "../../graphics/assetEngine.hpp"

class AssetTestApp : public al::DistributedApp {
public:
  AssetEngine assetEngine;
  AssetEngine* mAssetEngine;
  al::DistributedScene mDistributedScene;

  void onInit() override {
    mDistributedScene.verbose(true);
    mDistributedScene.registerSynthClass<AssetEngine>();
    this->registerDynamicScene(mDistributedScene);
    mAssetEngine = mDistributedScene.getVoice<AssetEngine>();
    mDistributedScene.triggerOn(mAssetEngine);
  }

  void onCreate() override {
    al::imguiInit();
  }

  void onAnimate(double dt) override {
    mDistributedScene.update(dt);
  }

  void onDraw(al::Graphics &g) override {
    g.clear(0.1); // Clear the screen with a dark gray background
    mDistributedScene.render(g);
  }
};

int main() {
  AssetTestApp app;
  app.start();
  return 0;
}