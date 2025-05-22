#include "al/app/al_DistributedApp.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "../../graphics/assetEngine.hpp"

class AssetTestApp : public al::DistributedApp {
public:
  al::DistributedScene mDistributedScene;
  AssetEngine* mAssetEngine = nullptr;

  void onInit() override {
    al::imguiInit();
    mDistributedScene.verbose(true);
    mDistributedScene.registerSynthClass<AssetEngine>();
    this->registerDynamicScene(mDistributedScene);
  }

  bool onKeyDown(const al::Keyboard& k) override {
    if (isPrimary() && k.key() == ' ') {
      if (mAssetEngine == nullptr) {
        mAssetEngine = mDistributedScene.getVoice<AssetEngine>();
        mDistributedScene.triggerOn(mAssetEngine);
      }
    }
    return true;
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