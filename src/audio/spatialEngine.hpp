#include <iostream>
#include <cmath>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Spherical.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_PickableManager.hpp"

#include "../../Gimmel/include/filter.hpp"

/**
 * @brief Converts spherical coordinates to Cartesian coordinates.
 *
 * This function takes spherical coordinates (azimuth, elevation, and radius)
 * and converts them to Cartesian coordinates (x, y, z) using the AlloSphere
 * convention. The azimuth and elevation angles are expected to be in degrees.
 *
 * @param azimuthDeg The azimuth angle in degrees, measured clockwise from the positive z-axis.
 * @param elevationDeg The elevation angle in degrees, measured from the xz-plane.
 * @param radius The radial distance from the origin.
 * @return A Vec3f object representing the Cartesian coordinates (x, y, z).
 */
al::Vec3f sphericalToCartesian(float azimuthDeg, float elevationDeg, float radius) {
  // Convert azimuth and elevation from degrees to radians
  constexpr float degToRad = M_PI / 180.0f; // constexpr to increase efficiency 
  float azimuthRad = azimuthDeg * degToRad;
  float elevationRad = elevationDeg * degToRad;
  
  // Calculate the Cartesian coordinates using AlloSphere convention
  float cosElevRad = cos(elevationRad);
  float x = radius * cosElevRad * sin(azimuthRad);
  float y = radius * sin(elevationRad);
  float z = -radius * cosElevRad * cos(azimuthRad);  // Right-handed system flip
  
  return al::Vec3f(x, y, z);
}

/**
 * @brief Converts Cartesian coordinates to spherical coordinates.
 * 
 * This function takes a 3D Cartesian coordinate (x, y, z) and converts it 
 * into spherical coordinates: azimuth angle (in degrees), elevation angle 
 * (in degrees), and radius (distance from the origin). The azimuth angle 
 * follows the AlloSphere convention, where the z-axis is treated as the 
 * forward direction.
 * 
 * @param cartesian A 3D vector representing the Cartesian coordinates (x, y, z).
 * @param azimuthDeg Output parameter for the azimuth angle in degrees.
 * @param elevationDeg Output parameter for the elevation angle in degrees.
 * @param radius Output parameter for the radius (distance from the origin).
 * 
 * @note If the radius is near zero (less than 1e-6), the azimuth and elevation 
 *       angles are set to 0 to avoid division by zero.
 */
void cartesianToSpherical(const al::Vec3f& cartesian, 
                          float& azimuthDeg, 
                          float& elevationDeg, 
                          float& radius) {
  float x = cartesian.x;
  float y = cartesian.y;
  float z = cartesian.z;
  
  radius = sqrt(x*x + y*y + z*z);
  
  if (radius < 1e-6) {
    azimuthDeg = 0.f;
    elevationDeg = 0.f;
    return;
  }
  
  constexpr float radToDeg = 180.0f / M_PI; // constexpr to increase efficiency 
  elevationDeg = asin(y / radius) * radToDeg;
  azimuthDeg = atan2(x, -z) * radToDeg;  // Note the negative z for AlloSphere convention
}

/**
 * @class SelectablePickable
 * @brief A class that extends PickableBB to add selection functionality.
 */
class SelectablePickable : public al::PickableBB {
public:
  bool selected = false;

  // set selected on hit
  bool onEvent(al::PickEvent e, al::Hit h) override {
    bool handled = PickableBB::onEvent(e, h);
    if (e.type == al::Pick && h.hit) { selected = true; }
    return handled;
  }
};

class PickableMesh : public al::VAOMesh, public SelectablePickable {
private:
public: 
  // init Mesh and Pickable in constructor
  PickableMesh() {
    addSphere(*this, 0.3);
    this->primitive(al::Mesh::LINE_STRIP);
    this->update();
    this->set(*this);
  }
};

/**
 * @class SpatialAgent
 * @brief A spatialized audio agent that generates a sine wave sound with adjustable parameters.
 * 
 * This class extends PositionedVoice and provides functionality for spatialized audio synthesis.
 * It supports configurable frequency, amplitude, gain, distance, size, and lifespan, with
 * distance-based attenuation and low-pass filtering for realistic sound propagation.
 */
class SpatialAgent : public al::PositionedVoice {
public:
  float baseAmplitude = 0.2f; // TODO
  float amplitude = 0.2f; // TODO
  float gain = 1.0f;
  float distance = 1.0f;
  float size = 1.0f;
  PickableMesh mPickableMesh;
  giml::OnePole<float> airFilter;

  al::Parameter mAzimuth{"Azimuth", "", 90.0, "", -180.0, 180.0};
  al::Parameter mElevation{"Elevation", "", 30.0, "", -90.0, 90.0};
  al::Parameter mDistance{"Distance", "", 8.0, "", 0.1, 20.0};
  al::Parameter mGain{"Gain", "", 1.0, "", 0.0, 2.0};
  al::ControlGUI mGui;


  void init() {
    mGui.init(5, 5, false);
    mGui.setTitle("Spatial Agent");
    mGui << mAzimuth << mElevation << mDistance << mGain;

    mPickableMesh.pose = al::Pose(sphericalToCartesian(mAzimuth.get(), 
                                  mElevation.get(), 
                                  mDistance.get()));

  }

  void onProcess(al::AudioIOData &io) override {
    while (io()) {
      //io.out(0) = io.in(0); // TODO (Channel Routing)
      io.out(0) = al::rnd::uniformS(); // default to noise 
    }
  }

  void set(float azimuthDeg, float elevationDeg, float distanceVal, 
           float sizeVal, float gainVal, unsigned int sampleRate) {
            
    al::Vec3f position = sphericalToCartesian(azimuthDeg, elevationDeg, distanceVal);
    setPose(al::Pose(position));
    baseAmplitude = 0.2f;
    distance = distanceVal;
    size = sizeVal;
    gain = gainVal;
    
    float distanceAttenuation = 1.0f / (distance * distance);
    distanceAttenuation = std::min(distanceAttenuation, 5.0f);
    amplitude = baseAmplitude * distanceAttenuation;

    float distCutoff = 20000.0f / (1.0f + distance * 0.8f);
    distCutoff = std::max(distCutoff, 300.0f);
    airFilter.setCutoff(distCutoff, sampleRate);
  }
};

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

  void onMouseMove(al::Graphics& g, const al::Mouse& m, int w, int h) {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return; // Don't pass to pickable manager if over GUI
      }
    }
    // If not over GUI, pass to pickable manager
    mPickableManager.onMouseMove(g, m, w, h);
  }

  void onMouseDown(al::Graphics& g, const al::Mouse& m, int w, int h) {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return;
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

    mPickableManager.onMouseDown(g, m, w, h);
  }

  void onMouseDrag(al::Graphics& g, const al::Mouse& m, int w, int h) {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return; // Don't pass to pickable manager if over GUI
      }
    }
    // If not over GUI, pass to pickable manager
    mPickableManager.onMouseDrag(g, m, w, h);
  }

  void onMouseUp(al::Graphics& g, const al::Mouse& m, int w, int h) {
    // Check if the mouse is over a GUI first
    for (auto agent : mAgents) {
      if (agent->mPickableMesh.selected && agent->mGui.usingInput()) {
        return; // Don't pass to pickable manager if over GUI
      }
    }
    // If not over GUI, pass to pickable manager
    mPickableManager.onMouseUp(g, m, w, h);
  }

};