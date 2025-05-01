

#define SAMPLERATE 44100
#define SPATIALIZER_TYPE Lbap // for playback in Allosphere
#define AUDIO_CONFIG 44100, 256, 60, 0 // for playback in Allosphere
#define SPEAKER_LAYOUT AlloSphereSpeakerLayoutCompensated() // for playback in Allosphere

#include "../../audio/spatialEngine.hpp"

struct MyApp : public App {
  DynamicScene scene;
  PickableManager pickableManager;
  //std::vector<std::unique_ptr<SpatialAgent>> mAgents; // TODO
  std::vector<SpatialAgent*> mAgents;
  
  // Flag to prevent feedback loop between parameter changes and pickable updates
  bool pickablesUpdatingParameters = false;
  
  // Fixed listener pose at origin
  Pose fixedListenerPose;

  void onCreate() override {
    auto speakers = SPEAKER_LAYOUT; 
    scene.setSpatializer<SPATIALIZER_TYPE>(speakers);
    scene.distanceAttenuation().law(ATTEN_NONE);
    
    // Set fixed listener pose at origin (0,0,0)
    fixedListenerPose = Pose(Vec3f(0, 0, 0));
    
    // Set up GUI windows
    imguiInit();
    
    // Add sound agents
    for (unsigned i = 0; i < 3; i++) {
      mAgents.push_back(scene.getVoice<SpatialAgent>()); // add an agent
    }

    for (auto agent : mAgents) {
      scene.triggerOn(agent); // trigger on
      pickableManager << agent->mPickable; // add pickable to manager
    }
    
    // Prepare the scene for audio rendering
    scene.prepare(audioIO());
    
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
      agent->mPickable.selected = false;
    }
  }

  void updateAgents() {
    for (auto agent : mAgents) {
      agent->set(agent->mAzimuth.get(), agent->mElevation.get(), agent->mDistance.get(), 
                 agent->size, agent->freq, agent->gain, 0);
    }
  }

  void updatePickablePositions() {
    for (auto agent : mAgents) {
      Vec3f position = sphericalToCartesian(agent->mAzimuth.get(), agent->mElevation.get(), agent->mDistance.get());
      agent->mPickable.pose = Pose(position);
    }
  }
  
  void onAnimate(double dt) override {
    // Disable navControl when GUI is in use
    // navControl().active(!sineGUI.usingInput() && !squareGUI.usingInput() && !pinkGUI.usingInput());
    
    // Update parameters from pickable positions
    // Only update parameters from pickables if GUI isn't being used

    // Check if the mouse is over a GUI first
    bool any = false;
    for (auto agent : mAgents) {
      if (agent->mPickable.selected && agent->mGui.usingInput()) {
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
        cartesianToSpherical(agent->mPickable.pose.get().pos(), az, el, dist);
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
    for (auto pickable : pickableManager.pickables()) {
      SelectablePickable* sp = dynamic_cast<SelectablePickable*>(pickable);
      if (sp && sp->selected) {
        for (auto otherPickable : pickableManager.pickables()) {
          SelectablePickable* otherSp = dynamic_cast<SelectablePickable*>(otherPickable);
          if (otherSp && otherSp != sp) {
            otherSp->selected = false; // Deselect all others
          }
        }
      }
    }

  }

  void onDraw(Graphics &g) override {
    g.clear();
    gl::depthTesting(true);
    
    // Draw coordinate reference axes
    g.lineWidth(2.0);
    Mesh axes;
    axes.primitive(Mesh::LINES);
    
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
    for (auto pickable : pickableManager.pickables()) {
      // Color based on which sound source
      int index = -1;
      for (int i = 0; i < pickableManager.pickables().size(); i++) {
        if (pickable == pickableManager.pickables()[i]) {
          index = i;
          break;
        }
      }
      
      SelectablePickable* sp = dynamic_cast<SelectablePickable*>(pickable);
      bool isSelected = sp && sp->selected;
      
      if (index == 0) { // Sine
        g.color(isSelected ? RGB(0.3, 1.0, 0.5) : RGB(0.1, 0.9, 0.3)); // Brighter green when selected
      } else if (index == 1) { // Square
        g.color(isSelected ? RGB(0.4, 0.6, 1.0) : RGB(0.2, 0.4, 1.0)); // Brighter blue when selected
      } else if (index == 2) { // Pink
        g.color(isSelected ? RGB(1.0, 0.5, 0.2) : RGB(0.9, 0.3, 0.1)); // Brighter orange when selected
      } else {
        g.color(1, 1, 1);
      }
      
      // Draw using lambda function like in the example
      pickable->draw(g, [&](Pickable &p) {
        auto &b = dynamic_cast<PickableBB &>(p);
        b.drawMesh(g);
      });
      
      // Draw line from origin to sound source
      g.lineWidth(1.0);
      g.color(0.5, 0.5, 0.5, 0.3);
      Mesh line;
      line.primitive(Mesh::LINES);
      line.vertex(0, 0, 0);
      line.vertex(pickable->pose.get().pos());
      g.draw(line);
    }
    
    // Draw the GUI - only for selected objects
    imguiBeginFrame();
    
    // Only show GUI for selected object
    for (auto agent : mAgents) {
      if (agent->mPickable.selected) {
        agent->mGui.draw(g);
      }
    }
    
    imguiEndFrame();
    imguiDraw();
  }

  void onSound(AudioIOData &io) override {
    // Use fixed listener pose at origin instead of camera position
    scene.listenerPose(fixedListenerPose);
    scene.render(io);
  }
  
  // Mouse event handling
  bool onMouseMove(const Mouse &m) override {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickable.selected && agent->mGui.usingInput()) {
        return true;
      }
    }
  
    // If not over GUI, pass to pickable manager
    pickableManager.onMouseMove(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDown(const Mouse &m) override {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickable.selected && agent->mGui.usingInput()) {
        return true;
      }
    }
    
    // If not over GUI, clear previous selections and pass to pickable manager
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
    
    pickableManager.onMouseDown(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDrag(const Mouse &m) override {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickable.selected && agent->mGui.usingInput()) {
        return true;
      }
    }
    
    // If not over GUI, pass to pickable manager
    pickableManager.onMouseDrag(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseUp(const Mouse &m) override {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickable.selected && agent->mGui.usingInput()) {
        return true;
      }
    }
    
    // If not over GUI, pass to pickable manager
    pickableManager.onMouseUp(graphics(), m, width(), height());
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