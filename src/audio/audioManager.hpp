#ifndef EOYS_AUDIO_MANAGER
#define EOYS_AUDIO_MANAGER

#include "al/scene/al_DistributedScene.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "spatialAgent.hpp"
#include "channelStrip.hpp"

class DistributedSceneWithInput : public al::DistributedScene {
public:
  /**
   * @brief Determines the number of input channels allocated for the internal
   * AudioIOData objects
   * @param channels
   *
   * Always call prepare() after calling this function. The changes are only
   * applied by prepare().
   */
  void setVoiceMaxInputChannels(uint16_t channels) {
    this->mVoiceMaxInputChannels = channels;
  }
};

/**
 * @brief AudioManager class for handling a number of SpatialAgents
 */
template <class TSynthVoice>
class AudioManager {
private:
  DistributedSceneWithInput mDistributedScene;
  al::PickableManager mPickableManager;
  std::vector<TSynthVoice*> mAgents;
  bool pickablesUpdatingParameters = false;
  al::Pose fixedListenerPose;
  int sampleRate = -1;

public:

  AudioManager() {
    mDistributedScene.setVoiceMaxInputChannels(8); // TODO don't hardcode this lol
    mDistributedScene.registerSynthClass<TSynthVoice>(); 
  }

  al::DistributedScene* scene() {
    return &mDistributedScene;
  }

  std::vector<TSynthVoice*>* agents() {
    return &mAgents;
  }

  void setListenerPose(const al::Pose& pose) {
    fixedListenerPose = pose;
  }

  void addAgent(const char name[]) {
    auto* newAgent = mDistributedScene.getVoice<TSynthVoice>();
    newAgent->setName(name); 
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
                 agent->mDistance.get(), agent->size, sampleRate);
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
    mDistributedScene.update();
    
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
    io.zeroOut(); // clear outputs... should be done?
    mDistributedScene.listenerPose(fixedListenerPose); // seg faults
    mDistributedScene.render(io);
  }

  void draw(al::Graphics& g) {
    mDistributedScene.render(g);
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