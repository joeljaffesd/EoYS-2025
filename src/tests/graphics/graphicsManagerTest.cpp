#include "al/app/al_DistributedApp.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "graphics/graphicsManager.hpp"

class MyApp : public al::DistributedApp {
private: 
  GraphicsManager mManager;

public:

  void onCreate() override {
    mManager.init();
    mManager.registerParameters(this->parameterServer());
  }

  bool onKeyDown(const al::Keyboard& k) override {
    if (this->isPrimary()) {
      if (k.key() == '[') {
        mManager.prevScene();
      } 
      else if (k.key() == ']') {
        mManager.nextScene();
      }
    }
    return true;
  }

  void onAnimate(double dt) override {
    mManager.update(dt);
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0);
    mManager.render(g);
  }

};

int main() {
  MyApp app;
  app.start();
  return 0;
}