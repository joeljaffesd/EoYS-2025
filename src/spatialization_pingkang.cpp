#include <iostream>
#include <cmath>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Spherical.hpp"
#include "al/scene/al_DynamicScene.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_PickableManager.hpp"

using namespace al;

#define SpatializerType AmbisonicsSpatializer // for Ambisonics
//#define SpatializerType Lbap // Layer-based amplitude panning

// ===================
// Helper: Convert Spherical to Cartesian using AlloSphere convention
// ===================
Vec3f sphericalToCartesian(float azimuthDeg, float elevationDeg, float radius) {
  // Convert azimuth and elevation from degrees to radians
  float azimuth_rad = azimuthDeg * M_PI / 180.0f;
  float elevation_rad = elevationDeg * M_PI / 180.0f;
  
  // Calculate the Cartesian coordinates using AlloSphere convention
  float x = radius * cos(elevation_rad) * sin(azimuth_rad);
  float y = radius * sin(elevation_rad);
  float z = -radius * cos(elevation_rad) * cos(azimuth_rad);  // Right-handed system flip
  
  return Vec3f(x, y, z);
}

// Convert Cartesian to Spherical coordinates
void cartesianToSpherical(const Vec3f& cartesian, float& azimuthDeg, float& elevationDeg, float& radius) {
  float x = cartesian.x;
  float y = cartesian.y;
  float z = cartesian.z;
  
  radius = sqrt(x*x + y*y + z*z);
  
  if (radius < 0.0001f) {
    azimuthDeg = 0.0f;
    elevationDeg = 0.0f;
    return;
  }
  
  elevationDeg = asin(y / radius) * 180.0f / M_PI;
  azimuthDeg = atan2(x, -z) * 180.0f / M_PI;  // Note the negative z for AlloSphere convention
}

// Simple low-pass filter for air absorption simulation
class LowpassFilter {
public:
  float cutoff = 20000.0f;
  float sampleRate = 44100.0f;
  float filterState = 0.0f;
  float filterCoeff = 0.0f;
  
  void setCutoff(float cutoffFreq) {
    cutoff = cutoffFreq;
    filterCoeff = std::exp(-2.0f * M_PI * cutoff / sampleRate);
  }
  
  float process(float input) {
    filterState = input * (1.0f - filterCoeff) + filterState * filterCoeff;
    return filterState;
  }
  
  void updateForDistance(float distance) {
    float distCutoff = 20000.0f / (1.0f + distance * 0.8f);
    distCutoff = std::max(distCutoff, 300.0f);
    setCutoff(distCutoff);
  }
};

// ===================
// Sine Tone Agent
// ===================
class MyAgent : public PositionedVoice {
public:
  float phase = 0.0f;
  float freq = 440.0f;
  float baseAmplitude = 0.2f;
  float amplitude = 0.2f;
  float gain = 1.0f;
  float distance = 1.0f;
  float size = 1.0f;
  unsigned int mLifeSpan = 0;
  
  LowpassFilter airFilter;

  void onProcess(AudioIOData &io) override {
    float phaseInc = 2.0f * M_PI * freq / io.framesPerSecond();
    while (io()) {
      float sample = amplitude * gain * sin(phase);
      sample = airFilter.process(sample);
      io.out(0) += sample;
      phase += phaseInc;
      if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
    }

    if (mLifeSpan > 0) {
      mLifeSpan--;
      if (mLifeSpan <= 0) {
        free();
      }
    }
  }

  void set(float azimuthDeg, float elevationDeg, float distanceVal, float sizeVal, float frequency, float gainVal, float lifeSpanFrames) {
    Vec3f position = sphericalToCartesian(azimuthDeg, elevationDeg, distanceVal);
    setPose(Pose(position));
    baseAmplitude = 0.2f;
    freq = frequency;
    distance = distanceVal;
    size = sizeVal;
    gain = gainVal;
    mLifeSpan = static_cast<unsigned int>(lifeSpanFrames);
    
    float distanceAttenuation = 1.0f / (distance * distance);
    distanceAttenuation = std::min(distanceAttenuation, 5.0f);
    amplitude = baseAmplitude * distanceAttenuation;
    
    airFilter.sampleRate = 44100.0f;
    airFilter.updateForDistance(distance);
  }

  void onTriggerOn() override {
    phase = 0.0f;
    airFilter.filterState = 0.0f;
  }
};

// ===================
// Pink Noise Agent
// ===================
class PinkAgent : public PositionedVoice {
public:
  float baseAmplitude = 0.15f;
  float amplitude = 0.15f;
  float gain = 1.0f;
  float distance = 1.0f;
  float size = 1.0f;
  unsigned int mLifeSpan = 0;
  
  float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;
  
  LowpassFilter airFilter;

  float generatePinkNoise(float white) {
    b0 = 0.99886f * b0 + white * 0.0555179f;
    b1 = 0.99332f * b1 + white * 0.0750759f;
    b2 = 0.96900f * b2 + white * 0.1538520f;
    b3 = 0.86650f * b3 + white * 0.3104856f;
    b4 = 0.55000f * b4 + white * 0.5329522f;
    b5 = -0.7616f * b5 - white * 0.0168980f;
    float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
    b6 = white * 0.115926f;
    return pink * 0.11f;
  }

  void onProcess(AudioIOData &io) override {
    while (io()) {
      float white = rnd::uniformS();
      float pink = generatePinkNoise(white);
      pink = airFilter.process(pink);
      io.out(0) += pink * amplitude * gain;
    }

    if (mLifeSpan > 0) {
      mLifeSpan--;
      if (mLifeSpan <= 0) {
        free();
      }
    }
  }

  void set(float azimuthDeg, float elevationDeg, float distanceVal, float sizeVal, float gainVal, float lifeSpanFrames) {
    Vec3f position = sphericalToCartesian(azimuthDeg, elevationDeg, distanceVal);
    setPose(Pose(position));
    baseAmplitude = 0.15f;
    distance = distanceVal;
    size = sizeVal;
    gain = gainVal;
    mLifeSpan = static_cast<unsigned int>(lifeSpanFrames);
    
    float distanceAttenuation = 1.0f / (distance * distance);
    distanceAttenuation = std::min(distanceAttenuation, 5.0f);
    amplitude = baseAmplitude * distanceAttenuation;
    
    airFilter.sampleRate = 44100.0f;
    airFilter.updateForDistance(distance);
  }

  void onTriggerOn() override {
    b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0.0f;
    airFilter.filterState = 0.0f;
  }
};

// ===================
// Square Wave Agent
// ===================
class SquareAgent : public PositionedVoice {
public:
  float phase = 0.0f;
  float freq = 440.0f;
  float baseAmplitude = 0.2f;
  float amplitude = 0.2f;
  float gain = 1.0f;
  float distance = 1.0f;
  float size = 1.0f;
  unsigned int mLifeSpan = 0;
  
  LowpassFilter airFilter;

  void onProcess(AudioIOData &io) override {
    float phaseInc = 2.0f * M_PI * freq / io.framesPerSecond();
    while (io()) {
      float sample = amplitude * gain * (sin(phase) >= 0 ? 1.0f : -1.0f);
      sample = airFilter.process(sample);
      io.out(0) += sample;
      phase += phaseInc;
      if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
    }

    if (mLifeSpan > 0) {
      mLifeSpan--;
      if (mLifeSpan <= 0) {
        free();
      }
    }
  }

  void set(float azimuthDeg, float elevationDeg, float distanceVal, float sizeVal, float frequency, float gainVal, float lifeSpanFrames) {
    Vec3f position = sphericalToCartesian(azimuthDeg, elevationDeg, distanceVal);
    setPose(Pose(position));
    baseAmplitude = 0.2f;
    freq = frequency;
    distance = distanceVal;
    size = sizeVal;
    gain = gainVal;
    mLifeSpan = static_cast<unsigned int>(lifeSpanFrames);
    
    float distanceAttenuation = 1.0f / (distance * distance);
    distanceAttenuation = std::min(distanceAttenuation, 5.0f);
    amplitude = baseAmplitude * distanceAttenuation;
    
    airFilter.sampleRate = 44100.0f;
    airFilter.updateForDistance(distance);
  }

  void onTriggerOn() override {
    phase = 0.0f;
    airFilter.filterState = 0.0f;
  }
};

// ==========================
// Main App
// ==========================
struct MyApp : public App {
  DynamicScene scene;
  PickableManager pickableManager;
  
  // GUI controls
  ControlGUI sineGUI;
  ControlGUI squareGUI;
  ControlGUI pinkGUI;
  
  // Parameters for sine wave
  Parameter sineAzimuth{"Azimuth", "", 90.0, "", -180.0, 180.0};
  Parameter sineElevation{"Elevation", "", 30.0, "", -90.0, 90.0};
  Parameter sineDistance{"Distance", "", 8.0, "", 0.1, 20.0};
  Parameter sineGain{"Gain", "", 1.0, "", 0.0, 2.0};
  
  // Parameters for square wave
  Parameter squareAzimuth{"Azimuth", "", 0.0, "", -180.0, 180.0};
  Parameter squareElevation{"Elevation", "", 0.0, "", -90.0, 90.0};
  Parameter squareDistance{"Distance", "", 3.0, "", 0.1, 20.0};
  Parameter squareGain{"Gain", "", 0.7, "", 0.0, 2.0};
  
  // Parameters for pink noise
  Parameter pinkAzimuth{"Azimuth", "", -90.0, "", -180.0, 180.0};
  Parameter pinkElevation{"Elevation", "", 30.0, "", -90.0, 90.0};
  Parameter pinkDistance{"Distance", "", 1.0, "", 0.1, 20.0};
  Parameter pinkGain{"Gain", "", 0.5, "", 0.0, 2.0};
  
  // Meshes
  VAOMesh sineMesh;
  VAOMesh squareMesh;
  VAOMesh pinkMesh;
  
  // The actual sound agents
  MyAgent* sineAgent = nullptr;
  SquareAgent* squareAgent = nullptr;
  PinkAgent* pinkAgent = nullptr;
  
  // Flag to prevent feedback loop between parameter changes and pickable updates
  bool pickablesUpdatingParameters = false;

  void onCreate() override {
    auto speakers = StereoSpeakerLayout(); // stereo is for StereoSpeakerLayout(), Allosphere's 54.1 is for AlloSphereSpeakerLayoutCompensated()
    scene.setSpatializer<SpatializerType>(speakers);
    scene.distanceAttenuation().law(ATTEN_NONE);
    
    // Create meshes for visual representation
    addSphere(sineMesh, 0.3);
    sineMesh.primitive(Mesh::LINE_STRIP);
    sineMesh.update();
    
    addSphere(squareMesh, 0.3);
    squareMesh.primitive(Mesh::LINE_STRIP);
    squareMesh.update();
    
    addSphere(pinkMesh, 0.3);
    pinkMesh.primitive(Mesh::LINE_STRIP);
    pinkMesh.update();
    
    // Set up GUI windows
    imguiInit();
    
    sineGUI.init(5, 5, false);
    sineGUI.setTitle("Sine Wave");
    sineGUI << sineAzimuth << sineElevation << sineDistance << sineGain;
    
    squareGUI.init(250, 5, false);
    squareGUI.setTitle("Square Wave");
    squareGUI << squareAzimuth << squareElevation << squareDistance << squareGain;
    
    pinkGUI.init(500, 5, false);
    pinkGUI.setTitle("Pink Noise");
    pinkGUI << pinkAzimuth << pinkElevation << pinkDistance << pinkGain;
    
    // Register parameter callbacks for GUI updates
    sineAzimuth.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    sineElevation.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    sineDistance.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    squareAzimuth.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    squareElevation.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    squareDistance.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    pinkAzimuth.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    pinkElevation.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    pinkDistance.registerChangeCallback([this](float value) {
      if (!pickablesUpdatingParameters) {
        updatePickablePositions();
      }
    });
    
    // Create pickable objects
    PickableBB* sinePickable = new PickableBB;
    sinePickable->set(sineMesh);
    sinePickable->pose = Pose(sphericalToCartesian(sineAzimuth.get(), sineElevation.get(), sineDistance.get()));
    pickableManager << sinePickable;
    
    PickableBB* squarePickable = new PickableBB;
    squarePickable->set(squareMesh);
    squarePickable->pose = Pose(sphericalToCartesian(squareAzimuth.get(), squareElevation.get(), squareDistance.get()));
    pickableManager << squarePickable;
    
    PickableBB* pinkPickable = new PickableBB;
    pinkPickable->set(pinkMesh);
    pinkPickable->pose = Pose(sphericalToCartesian(pinkAzimuth.get(), pinkElevation.get(), pinkDistance.get()));
    pickableManager << pinkPickable;
    
    // Start sound agents
    sineAgent = scene.getVoice<MyAgent>();
    if (sineAgent) {
      sineAgent->set(sineAzimuth.get(), sineElevation.get(), sineDistance.get(), 
                   1.0f, 440.0f, sineGain.get(), 0);
      scene.triggerOn(sineAgent);
    }
    
    squareAgent = scene.getVoice<SquareAgent>();
    if (squareAgent) {
      squareAgent->set(squareAzimuth.get(), squareElevation.get(), squareDistance.get(), 
                     1.0f, 330.0f, squareGain.get(), 0);
      scene.triggerOn(squareAgent);
    }
    
    pinkAgent = scene.getVoice<PinkAgent>();
    if (pinkAgent) {
      pinkAgent->set(pinkAzimuth.get(), pinkElevation.get(), pinkDistance.get(), 
                    1.0f, pinkGain.get(), 0);
      scene.triggerOn(pinkAgent);
    }
    
    // Prepare the scene for audio rendering
    scene.prepare(audioIO());
    
    // Set up camera
    nav().pos(0, 0, 10);
    navControl().useMouse(false);
    
    std::cout << "3D Sound Spatialization with GUI and Pickable Objects:" << std::endl;
    std::cout << "  1. Use sliders to control sound source positions" << std::endl;
    std::cout << "  2. Click and drag objects to move them in space" << std::endl;
    std::cout << "  3. Press SPACE to reset objects to original positions" << std::endl;
  }
  
  // Update pickable positions based on current parameter values
  void updatePickablePositions() {
    auto pickables = pickableManager.pickables();
    
    // Update sine wave pickable
    if (pickables.size() > 0) {
      Vec3f position = sphericalToCartesian(sineAzimuth.get(), sineElevation.get(), sineDistance.get());
      pickables[0]->pose = Pose(position);
    }
    
    // Update square wave pickable
    if (pickables.size() > 1) {
      Vec3f position = sphericalToCartesian(squareAzimuth.get(), squareElevation.get(), squareDistance.get());
      pickables[1]->pose = Pose(position);
    }
    
    // Update pink noise pickable
    if (pickables.size() > 2) {
      Vec3f position = sphericalToCartesian(pinkAzimuth.get(), pinkElevation.get(), pinkDistance.get());
      pickables[2]->pose = Pose(position);
    }
    
    updateAudioAgents();
  }
  
  // Update audio agents with current parameters
  void updateAudioAgents() {
    if (sineAgent) {
      sineAgent->set(sineAzimuth.get(), sineElevation.get(), sineDistance.get(), 
                   1.0f, 440.0f, sineGain.get(), 0);
    }
    
    if (squareAgent) {
      squareAgent->set(squareAzimuth.get(), squareElevation.get(), squareDistance.get(), 
                     1.0f, 330.0f, squareGain.get(), 0);
    }
    
    if (pinkAgent) {
      pinkAgent->set(pinkAzimuth.get(), pinkElevation.get(), pinkDistance.get(), 
                    1.0f, pinkGain.get(), 0);
    }
  }
  
  void onAnimate(double dt) override {
    // Disable navControl when GUI is in use
    navControl().active(!sineGUI.usingInput() && !squareGUI.usingInput() && !pinkGUI.usingInput());
    
    // Update parameters from pickable positions
    auto pickables = pickableManager.pickables();
    
    // Only update parameters from pickables if GUI isn't being used
    if (!sineGUI.usingInput() && !squareGUI.usingInput() && !pinkGUI.usingInput()) {
      pickablesUpdatingParameters = true;
      
      if (pickables.size() > 0) {
        float az, el, dist;
        cartesianToSpherical(pickables[0]->pose.get().pos(), az, el, dist);
        sineAzimuth.set(az);
        sineElevation.set(el);
        sineDistance.set(dist);
      }
      
      if (pickables.size() > 1) {
        float az, el, dist;
        cartesianToSpherical(pickables[1]->pose.get().pos(), az, el, dist);
        squareAzimuth.set(az);
        squareElevation.set(el);
        squareDistance.set(dist);
      }
      
      if (pickables.size() > 2) {
        float az, el, dist;
        cartesianToSpherical(pickables[2]->pose.get().pos(), az, el, dist);
        pinkAzimuth.set(az);
        pinkElevation.set(el);
        pinkDistance.set(dist);
      }
      
      pickablesUpdatingParameters = false;
    }
    
    // Always update audio agents
    updateAudioAgents();
  }

  void onDraw(Graphics &g) override {
    g.clear();
    gl::depthTesting(true);
    
    // Set listener pose
    scene.listenerPose(nav());
    
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
      
      if (index == 0) {
        g.color(0.1, 0.9, 0.3); // Green for sine
      } else if (index == 1) {
        g.color(0.2, 0.4, 1.0); // Blue for square
      } else if (index == 2) {
        g.color(0.9, 0.3, 0.1); // Orange for pink
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
    
    // Draw the GUI
    imguiBeginFrame();
    sineGUI.draw(g);
    squareGUI.draw(g);
    pinkGUI.draw(g);
    imguiEndFrame();
    imguiDraw();
  }

  void onSound(AudioIOData &io) override {
    scene.render(io);
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {
      // Reset all positions to defaults
      auto pickables = pickableManager.pickables();
      
      // Reset sine wave
      sineAzimuth.set(90.0);
      sineElevation.set(30.0);
      sineDistance.set(8.0);
      sineGain.set(1.0);
      if (pickables.size() > 0) {
        pickables[0]->pose = Pose(sphericalToCartesian(90.0, 30.0, 8.0));
      }
      
      // Reset square wave
      squareAzimuth.set(0.0);
      squareElevation.set(0.0);
      squareDistance.set(3.0);
      squareGain.set(0.7);
      if (pickables.size() > 1) {
        pickables[1]->pose = Pose(sphericalToCartesian(0.0, 0.0, 3.0));
      }
      
      // Reset pink noise
      pinkAzimuth.set(-90.0);
      pinkElevation.set(30.0);
      pinkDistance.set(1.0);
      pinkGain.set(0.5);
      if (pickables.size() > 2) {
        pickables[2]->pose = Pose(sphericalToCartesian(-90.0, 30.0, 1.0));
      }
      
      updateAudioAgents();
      return true;
    }
    return false;
  }
  
  // Handle pickable interaction - EXACTLY as in the example
  bool onMouseMove(const Mouse &m) override {
    pickableManager.onMouseMove(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDown(const Mouse &m) override {
    pickableManager.onMouseDown(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDrag(const Mouse &m) override {
    pickableManager.onMouseDrag(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseUp(const Mouse &m) override {
    pickableManager.onMouseUp(graphics(), m, width(), height());
    return true;
  }
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.title("3D Sound Spatialization");
  app.configureAudio(44100, 256, 2, 0);   // for stereo, app.configureAudio(44100, 256, 2, 0); 60 for allosphere
  app.start();
  return 0;
}