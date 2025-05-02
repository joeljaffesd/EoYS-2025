#define SPATIALIZER_TYPE al::AmbisonicsSpatializer // for Ambisonics
#ifndef SPATIALIZER_TYPE
#define SPATIALIZER_TYPE al::Lbap // for playback in Allosphere
#endif

#define AUDIO_CONFIG 48000, 128, 2, 1 // for lap/desktop computer 
#ifndef AUDIO_CONFIG
#define AUDIO_CONFIG 44100, 256, 60, 0 // for playback in Allosphere
#endif

#define SPEAKER_LAYOUT al::StereoSpeakerLayout() // for lap/desktop computer 
#ifndef SPEAKER_LAYOUT
#define SPEAKER_LAYOUT al::AlloSphereSpeakerLayoutCompensated() // for playback in Allosphere
#endif

#include "al/scene/al_DistributedScene.hpp"
#include "../../audio/spatialEngine.hpp"

struct MyApp : public al::App {
  al::DistributedScene mDistributedScene;
  al::PickableManager mPickableManager;
  //std::vector<std::unique_ptr<SpatialAgent>> mAgents; // TODO (smart pointers)
  std::vector<SpatialAgent*> mAgents;
  
  // Flag to prevent feedback loop between parameter changes and pickable updates
  bool pickablesUpdatingParameters = false;
  
  // Fixed listener pose at origin
  al::Pose fixedListenerPose;

  void onCreate() override {
    auto speakers = SPEAKER_LAYOUT; 
    mDistributedScene.setSpatializer<SPATIALIZER_TYPE>(speakers);
    mDistributedScene.distanceAttenuation().law(al::ATTEN_NONE);
    
    // Set fixed listener pose at origin (0,0,0)
    fixedListenerPose = al::Pose(al::Vec3f(0, 0, 0));
    
    // Set up GUI windows
    al::imguiInit();
    
    // Add sound agents
    for (unsigned i = 0; i < 3; i++) {
      mAgents.push_back(mDistributedScene.getVoice<SpatialAgent>()); // add an agent
    }

    for (auto agent : mAgents) {
      mDistributedScene.triggerOn(agent); // trigger on
      mPickableManager << agent->mPickableMesh; // add pickable to manager
    }
    
    // Prepare the mDistributedScene for audio rendering
    mDistributedScene.prepare(audioIO());
    
    // Set up camera - still movable but doesn't affect audio
    nav().pos(0, 0, 10);
    
    std::cout << "3D Sound Spatialization with GUI and Pickable Objects:" << std::endl;
    std::cout << "  1. Click on a sound source to show its control panel" << std::endl;
    std::cout << "  2. Click and drag objects to move them in space" << std::endl;
    std::cout << "  3. Press SPACE to reset objects to original positions" << std::endl;
    std::cout << "  4. Camera can be moved with arrow keys (view only, doesn't affect audio)" << std::endl;
  }
  
  // Clear selection on all pickables
  void clearAllSelections() {
    for (auto agent : mAgents) {
      agent->mPickableMesh.selected = false;
    }
  }

  // TODO: Move to `update()` call in mDistributedScene
  void updateAgents() {
    for (auto agent : mAgents) {
      agent->set(agent->mAzimuth.get(), agent->mElevation.get(), agent->mDistance.get(), 
                 agent->size, agent->gain, int(this->audioIO().framesPerSecond()));
    }
  }

  // TODO: Move to `update()` call in mDistributedScene
  void updatePickablePositions() {
    for (auto agent : mAgents) {
      al::Vec3f position = sphericalToCartesian(agent->mAzimuth.get(), agent->mElevation.get(), agent->mDistance.get());
      agent->mPickableMesh.pose = al::Pose(position);
    }
  }
  
  void onAnimate(double dt) override {
    // Disable navControl when GUI is in use
    // navControl().active(!sineGUI.usingInput() && !squareGUI.usingInput() && !pinkGUI.usingInput());
    
    // Update parameters from pickable positions
    // Only update parameters from pickables if GUI isn't being used

    // Check if the mouse is over a GUI first
    // TODO: Move to `update()` call in mDistributedScene
    bool any = false;
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        any = true;
        break;
      }
    }

    if (any) {
      pickablesUpdatingParameters = false;
    } else {
      pickablesUpdatingParameters = true;
      for (auto agent : mAgents) {
        float az, el, dist;
        cartesianToSpherical(agent->mPickableMesh.pose.get().pos(), az, el, dist);
        agent->mAzimuth.set(az);
        agent->mElevation.set(el);
        agent->mDistance.set(dist);
      }
      pickablesUpdatingParameters = false;
    }

    // Always update agents
    updateAgents();
    updatePickablePositions();
    
    // When clicking a new object, deselect all others - mutually exclusive selection
    for (auto pickable : mPickableManager.pickables()) {
      SelectablePickable* sp = dynamic_cast<SelectablePickable*>(pickable);
      if (sp && sp->selected) {
        for (auto otherPickable : mPickableManager.pickables()) {
          SelectablePickable* otherSp = dynamic_cast<SelectablePickable*>(otherPickable);
          if (otherSp && otherSp != sp) {
            otherSp->selected = false; // Deselect all others
          }
        }
      }
    }

  }

  void onDraw(al::Graphics &g) override {
    g.clear();
    al::gl::depthTesting(true);
    
    // Draw coordinate reference axes
    g.lineWidth(2.0);
    al::Mesh axes;
    axes.primitive(al::Mesh::LINES);
    
    // X axis (red)
    g.color(1, 0, 0);
    axes.vertex(0, 0, 0);
    axes.vertex(1, 0, 0);
    
    // Y axis (green)
    g.color(0, 1, 0);
    axes.vertex(0, 0, 0);
    axes.vertex(0, 1, 0);
    
    // Z axis (blue)
    g.color(0, 0, 1);
    axes.vertex(0, 0, 0);
    axes.vertex(0, 0, 1);
    
    g.draw(axes);
    
    // Draw pickable objects - Using the exact method from example
    // TODO: Move to `update()` call in mDistributedScene
    for (auto pickable : mPickableManager.pickables()) {
      // Color based on which sound source
      int index = -1;
      for (int i = 0; i < mPickableManager.pickables().size(); i++) {
        if (pickable == mPickableManager.pickables()[i]) {
          index = i;
          break;
        }
      }
      
      // TODO: Move to `update()` call in mDistributedScene
      SelectablePickable* sp = dynamic_cast<SelectablePickable*>(pickable);
      bool isSelected = sp && sp->selected;
      
      // TODO: Move to `update()` call in mDistributedScene
      if (index == 0) { // Sine
        g.color(isSelected ? al::RGB(0.3, 1.0, 0.5) : al::RGB(0.1, 0.9, 0.3)); // Brighter green when selected
      } else if (index == 1) { // Square
        g.color(isSelected ? al::RGB(0.4, 0.6, 1.0) : al::RGB(0.2, 0.4, 1.0)); // Brighter blue when selected
      } else if (index == 2) { // Pink
        g.color(isSelected ? al::RGB(1.0, 0.5, 0.2) : al::RGB(0.9, 0.3, 0.1)); // Brighter orange when selected
      } else {
        g.color(1, 1, 1);
      }
      
      // Draw using lambda function like in the example
      // TODO: Move to `update()` call in mDistributedScene
      pickable->draw(g, [&](al::Pickable &p) {
        auto &b = dynamic_cast<al::PickableBB &>(p);
        b.drawMesh(g);
      });
      
      // Draw line from origin to sound source
      // TODO: Move to `update()` call in mDistributedScene
      g.lineWidth(1.0);
      g.color(0.5, 0.5, 0.5, 0.3);
      al::Mesh line;
      line.primitive(al::Mesh::LINES);
      line.vertex(0, 0, 0);
      line.vertex(pickable->pose.get().pos());
      g.draw(line);
    }
    
    // Draw the GUI - only for selected objects
    al::imguiBeginFrame();
    
    // Only show GUI for selected object
    // TODO: Move to `update()` call in mDistributedScene
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected) {
        agent->mGui.draw(g);
      }
    }
    
    al::imguiEndFrame();
    al::imguiDraw();
  }

  void onSound(al::AudioIOData &io) override {
    // Use fixed listener pose at origin instead of camera position
    mDistributedScene.listenerPose(fixedListenerPose);
    mDistributedScene.render(io);
  }
  
  // Mouse event handling
  bool onMouseMove(const al::Mouse& m) override {
    // Check if the mouse is over a GUI first
    // TODO: Move to `update()` call in mDistributedScene
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return true;
      }
    }
  
    // If not over GUI, pass to pickable manager
    // maybe integrate mPickableManager into an AudioManager class,
    // that inherits from DistributedScene?
    mPickableManager.onMouseMove(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDown(const al::Mouse& m) override {
    // Check if the mouse is over a GUI first
    // TODO: Move to `update()` call in mDistributedScene
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return true;
      }
    }
    
    // If not over GUI, clear previous selections and pass to pickable manager
    // TODO: Move to `update()` call in mDistributedScene
    bool overGUI = false; 
    for (auto agent : mAgents) {
      if (agent->mGui.usingInput()) {
        overGUI = true;
        break;
      }
    }

    if (!overGUI) {
      clearAllSelections();
    }

    // maybe integrate mPickableManager into an AudioManager class,
    // that inherit from DistributedScene?
    mPickableManager.onMouseDown(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDrag(const al::Mouse& m) override {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return true;
      }
    }
    
    // If not over GUI, pass to pickable manager
    // maybe integrate mPickableManager into an AudioManager class,
    // that inherit from DistributedScene?
    mPickableManager.onMouseDrag(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseUp(const al::Mouse& m) override {
    // Check if the mouse is over a GUI first
    // TODO: Move to `update()` call in mDistributedScene
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return true;
      }
    }
    
    // If not over GUI, pass to pickable manager
    // maybe integrate mPickableManager into an AudioManager class,
    // that inherit from DistributedScene?
    mPickableManager.onMouseUp(graphics(), m, width(), height());
    return true;
  }
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.title("3D Sound Spatialization");
  app.configureAudio(AUDIO_CONFIG);
  app.start();
  return 0;
}