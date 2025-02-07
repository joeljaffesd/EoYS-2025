// Joel A. Jaffe 2025-02-06

#include "al/app/al_App.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/math/al_Random.hpp"
#include "al/app/al_GUIDomain.hpp"
using namespace al;

// define a struct that inherits from al::App
struct GraphicsKickstart : public al::App {
  ParameterBool audioToggle{"audioToggle", "audioToggle", false}; // muted by default
  Mesh mMesh{Mesh::Primitive::POINTS}; // init mesh with draw style

  // Called on app start
  void onInit() override { 
    std::cout << "onInit()" << std::endl;
    
    // init gui
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(audioToggle); // <- add parameter to GUI

    // add 4 vertices to mesh
    for (int i = 0; i < 4; i++) {
      Vec3f tempVert = rnd::uniformS(1.f);
      HSV tempColor = HSV(rnd::uniform(1.f));
      mMesh.vertex(tempVert);
      mMesh.color(tempColor);
    }
  }

  // Called when graphics context is available
  void onCreate() override { 
    std::cout << "onCreate()" << std::endl;
  }

  // Animation callback
  void onAnimate(double dt) override {
    for (int i = 0; i < mMesh.vertices().size(); i++) {
      Vec3f displacement = rnd::uniformS(0.01f);
      mMesh.vertices()[i] += displacement;
    }
  }

  // Grapics callback
  void onDraw(Graphics& g) override {
    g.clear(0);
    g.pointSize(20);
    g.meshColor();
    g.draw(mMesh);
  }

  // Audio callback
  void onSound(AudioIOData &io) override { 
    // for now, set all channel outputs == input channel 0 input
    // watch out for feedback!
    for (int sample = 0; sample < io.framesPerBuffer(); sample++) {
      for (int channel = 0; channel < io.channelsOut(); channel++) {
        if (audioToggle) {
          io.out(channel, sample) = io.in(0, sample); 
        } else {
          io.out(channel, sample) = 0.f;
        }
      }
    }
  }

  // Keyboard callback 
  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == 'm') {
      std::cout << "You pressed m" << std::endl;  
    } else {
      std::cout << "You pressed something other than m" << std::endl;  
    }
    return true;
  }

  // OSC message callback
  void onMessage(osc::Message &m) override { 
    m.print();
  }

};

int main() {
  GraphicsKickstart app;
  app.start();
  return 0;
}