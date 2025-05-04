#ifndef EOYS_AUDIO_MANAGER
#define EOYS_AUDIO_MANAGER

#include "al/scene/al_DistributedScene.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "spatialAgent.hpp"

/**
 * @brief AudioManager class for handling a number of SpatialAgents
 */
class AudioManager {
private:
  al::DistributedScene mDistributedScene;
  al::PickableManager mPickableManager;
  std::vector<SpatialAgent*> mAgents;
  bool pickablesUpdatingParameters = false;
  al::Pose fixedListenerPose;
  int sampleRate = -1;

public:

  al::DistributedScene* scene() {
    return &mDistributedScene;
  }

  // TODO
  // void setSpatializer() {}

  void setListenerPose(const al::Pose& pose) {
    fixedListenerPose = pose;
  }

  void addAgent() {
    auto* newAgent = mDistributedScene.getVoice<SpatialAgent>();
    mAgents.push_back(newAgent);
    mPickableManager << newAgent->mPickableMesh; // add pickable to manager
  }

  void prepare(al::AudioIO& audioIO) {
    // Prepare the mDistributedScene for audio rendering
    mDistributedScene.prepare(audioIO);
    sampleRate = int(audioIO.framesPerSecond());
    for (auto agent : mAgents) {
      mDistributedScene.triggerOn(agent);
    }
    // what else do we need to do here?
    // ... 
  }

  // Clear selection on all pickables
  void clearAllSelections() {
    for (auto agent : mAgents) {
      agent->mPickableMesh.selected = false;
    }
  }

  // TODO: put sampleRate in the constructor for agents
  void updateAgents() {
    for (auto agent : mAgents) {
      agent->set(agent->mAzimuth.get(), agent->mElevation.get(),
                 agent->mDistance.get(), agent->size, agent->gain, sampleRate);
    }
  }

  void updatePickablePositions() {
    for (auto agent : mAgents) {
      al::Vec3f position = sphericalToCartesian(agent->mAzimuth.get(), agent->mElevation.get(), agent->mDistance.get());
      agent->mPickableMesh.pose = al::Pose(position);
    }
  }

  void update() {
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

  void processAudio(al::AudioIOData& io) {
    mDistributedScene.listenerPose(fixedListenerPose);
    mDistributedScene.render(io);
  }

  void draw(al::Graphics& g) {
    // Draw pickable objects - Using the exact method from example
    for (auto pickable : mPickableManager.pickables()) {
      // Color based on which sound source
      int index = -1;
      for (int i = 0; i < mPickableManager.pickables().size(); i++) {
        if (pickable == mPickableManager.pickables()[i]) {
          index = i;
          break;
        }
      }
      
      SelectablePickable* sp = dynamic_cast<SelectablePickable*>(pickable);
      bool isSelected = sp && sp->selected;
      
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
      pickable->draw(g, [&](al::Pickable &p) {
        auto &b = dynamic_cast<al::PickableBB &>(p);
        b.drawMesh(g);
      });
      
      // Draw line from origin to sound source
      g.lineWidth(1.0);
      g.color(0.5, 0.5, 0.5, 0.3);
      al::Mesh line;
      line.primitive(al::Mesh::LINES);
      line.vertex(0, 0, 0);
      line.vertex(pickable->pose.get().pos());
      g.draw(line);
    }
  }

  void drawGUI(al::Graphics& g) {
    // Draw the GUI - only for selected objects
    al::imguiBeginFrame();
    
    // Only show GUI for selected object
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected) {
        agent->mGui.draw(g);
      }
    }
    
    al::imguiEndFrame();
    al::imguiDraw();
  }

  // helper function for mouse events
  bool mouseOverGUI() {
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return true; // Mouse is over GUI
      }
    }
    return false; // Mouse is not over GUI
  }

  void onMouseMove(al::Graphics& g, const al::Mouse& m, int w, int h) {
    // Check if the mouse is over a GUI first
    if (mouseOverGUI()) { return; }
    // If not over GUI, pass to pickable manager
    mPickableManager.onMouseMove(g, m, w, h);
  }

  void onMouseDown(al::Graphics& g, const al::Mouse& m, int w, int h) {
    // Check if the mouse is over a GUI first
    if (mouseOverGUI()) { return; }

    // If not over GUI, clear previous selections and pass to pickable manager
    bool overGUI = false; 
    for (auto agent : mAgents) {
      if (agent->mGui.usingInput()) {
        overGUI = true;
        break;
      }
    }

    if (!overGUI) { clearAllSelections(); }

    mPickableManager.onMouseDown(g, m, w, h);
  }

  void onMouseDrag(al::Graphics& g, const al::Mouse& m, int w, int h) {
    // Check if the mouse is over a GUI first
    if (mouseOverGUI()) { return; }
    // If not over GUI, pass to pickable manager
    mPickableManager.onMouseDrag(g, m, w, h);
  }

  void onMouseUp(al::Graphics& g, const al::Mouse& m, int w, int h) {
    // Check if the mouse is over a GUI first
    if (mouseOverGUI()) { return; }
    // If not over GUI, pass to pickable manager
    mPickableManager.onMouseUp(g, m, w, h);
  }

};

#endif // EOYS_AUDIO_MANAGER