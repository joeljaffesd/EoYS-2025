#include <iostream>
#include <cmath>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Spherical.hpp"
#include "al/scene/al_DynamicScene.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"

using namespace al;

#define SpatializerType AmbisonicsSpatializer

// ===================
// Helper: Convert Spherical to Cartesian using AlloLib
// ===================
Vec3f sphericalToCartesian(float azimuthDeg, float elevationDeg, float radius) {
  float az = azimuthDeg * M_PI / 180.0f;
  float el = elevationDeg * M_PI / 180.0f;
  float coords[3] = {radius, az, el};
  sphericalToCart(coords);  // modifies in place: r, az, el -> x, y, z
  return Vec3f(coords[0], coords[1], coords[2]);
}

// Simple low-pass filter for air absorption simulation
class LowpassFilter {
public:
  float cutoff = 20000.0f;
  float sampleRate = 44100.0f;
  float filterState = 0.0f;
  float filterCoeff = 0.0f;
  
  // Update filter coefficient based on cutoff frequency
  void setCutoff(float cutoffFreq) {
    cutoff = cutoffFreq;
    // Simple one-pole low-pass filter coefficient
    filterCoeff = std::exp(-2.0f * M_PI * cutoff / sampleRate);
  }
  
  // Process a single sample
  float process(float input) {
    filterState = input * (1.0f - filterCoeff) + filterState * filterCoeff;
    return filterState;
  }
  
  // Set the filter based on distance - enhanced effect
  void updateForDistance(float distance) {
    // Calculate cutoff frequency based on distance
    // Enhanced distance effect - stronger reduction with distance
    float distCutoff = 20000.0f / (1.0f + distance * 0.8f);
    
    // Limit minimum cutoff
    distCutoff = std::max(distCutoff, 300.0f);
    
    setCutoff(distCutoff);
    
    std::cout << "Distance: " << distance << "m, Cutoff: " << distCutoff << "Hz" << std::endl;
  }
};

// ===================
// Sine Tone Agent
// ===================
class MyAgent : public PositionedVoice {
public:
  float phase = 0.0f;
  float freq = 440.0f;
  float baseAmplitude = 0.2f;   // the intrinsic loudness of the sound source.
  float amplitude = 0.2f;
  float gain = 1.0f;        // New gain parameter
  float distance = 1.0f;
  float size = 1.0f;        // Size parameter (visual size)
  unsigned int mLifeSpan = 0;
  
  // Add air absorption filter
  LowpassFilter airFilter;

  void onProcess(AudioIOData &io) override {
    float phaseInc = 2.0f * M_PI * freq / io.framesPerSecond();
    while (io()) {
      // Generate sine wave
      float sample = amplitude * gain * sin(phase);
      
      // Apply air absorption filter
      sample = airFilter.process(sample);
      
      io.out(0) += sample;
      phase += phaseInc;
      if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
    }

    mLifeSpan--;
    if (mLifeSpan <= 0) {
      free();
    }
  }

  void onProcess(Graphics &g) override {
    Mesh *sharedMesh = static_cast<Mesh *>(userData());
    g.pushMatrix();
    gl::polygonMode(GL_LINE);
    g.color(0.1, 0.9, 0.3);
    g.translate(pose().pos());
    g.scale(size);
    g.draw(*sharedMesh);
    g.popMatrix();
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
    
    // Apply manual distance attenuation
    // Inverse square law with strong effect: 1/(distance^2)
    float distanceAttenuation = 1.0f / (distance * distance);
    // Clamp to prevent extremely loud sounds when close
    distanceAttenuation = std::min(distanceAttenuation, 5.0f);
    amplitude = baseAmplitude * distanceAttenuation;
    
    // Update air absorption filter based on distance
    airFilter.sampleRate = 44100.0f; // Should match your app's sample rate
    airFilter.updateForDistance(distance);
    
    std::cout << "Sine wave at az=" << azimuthDeg << "°, el=" << elevationDeg 
              << "°, dist=" << distanceVal << "m, size=" << sizeVal
              << ", gain=" << gainVal << ", amplitude=" << amplitude 
              << " (attenuation=" << distanceAttenuation << ")" << std::endl;
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
  float gain = 1.0f;        // New gain parameter
  float distance = 1.0f;
  float size = 1.0f;        // Size parameter (visual size)
  unsigned int mLifeSpan = 0;
  
  // Pink noise generator variables
  float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;
  
  // Air absorption filter
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
      // Generate pink noise
      float white = rnd::uniformS();
      float pink = generatePinkNoise(white);
      
      // Apply air absorption filter
      pink = airFilter.process(pink);
      
      io.out(0) += pink * amplitude * gain;
    }

    mLifeSpan--;
    if (mLifeSpan <= 0) {
      free();
    }
  }

  void onProcess(Graphics &g) override {
    Mesh *sharedMesh = static_cast<Mesh *>(userData());
    g.pushMatrix();
    gl::polygonMode(GL_LINE);
    g.color(0.9, 0.3, 0.1);
    g.translate(pose().pos());
    g.scale(size);
    g.draw(*sharedMesh);
    g.popMatrix();
  }

  void set(float azimuthDeg, float elevationDeg, float distanceVal, float sizeVal, float gainVal, float lifeSpanFrames) {
    Vec3f position = sphericalToCartesian(azimuthDeg, elevationDeg, distanceVal);
    setPose(Pose(position));
    baseAmplitude = 0.15f;
    distance = distanceVal;
    size = sizeVal;
    gain = gainVal;
    mLifeSpan = static_cast<unsigned int>(lifeSpanFrames);
    
    // Apply manual distance attenuation
    // Inverse square law with strong effect: 1/(distance^2)
    float distanceAttenuation = 1.0f / (distance * distance);
    // Clamp to prevent extremely loud sounds when close
    distanceAttenuation = std::min(distanceAttenuation, 5.0f);
    amplitude = baseAmplitude * distanceAttenuation;
    
    // Update air absorption filter based on distance
    airFilter.sampleRate = 44100.0f;
    airFilter.updateForDistance(distance);
    
    std::cout << "Pink noise at az=" << azimuthDeg << "°, el=" << elevationDeg 
              << "°, dist=" << distanceVal << "m, size=" << sizeVal
              << ", gain=" << gainVal << ", amplitude=" << amplitude 
              << " (attenuation=" << distanceAttenuation << ")" << std::endl;
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
    float gain = 1.0f;        // New gain parameter
    float distance = 1.0f;
    float size = 1.0f;        // Size parameter (visual size)
    unsigned int mLifeSpan = 0;
    
    // Air absorption filter
    LowpassFilter airFilter;
  
    void onProcess(AudioIOData &io) override {
      float phaseInc = 2.0f * M_PI * freq / io.framesPerSecond();
      while (io()) {
        // Generate square wave
        float sample = amplitude * gain * (sin(phase) >= 0 ? 1.0f : -1.0f);
        
        // Apply air absorption filter
        sample = airFilter.process(sample);
        
        io.out(0) += sample;
        phase += phaseInc;
        if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
      }
  
      mLifeSpan--;
      if (mLifeSpan <= 0) {
        free();
      }
    }
  
    void onProcess(Graphics &g) override {
      Mesh *sharedMesh = static_cast<Mesh *>(userData());
      g.pushMatrix();
      gl::polygonMode(GL_LINE);
      g.color(0.2, 0.4, 1.0);
      g.translate(pose().pos());
      g.scale(size);
      g.draw(*sharedMesh);
      g.popMatrix();
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
      
      // Apply manual distance attenuation
      // Inverse square law with strong effect: 1/(distance^2)
      float distanceAttenuation = 1.0f / (distance * distance);
      // Clamp to prevent extremely loud sounds when close
      distanceAttenuation = std::min(distanceAttenuation, 5.0f);
      amplitude = baseAmplitude * distanceAttenuation;
      
      // Update air absorption filter based on distance
      airFilter.sampleRate = 44100.0f;
      airFilter.updateForDistance(distance);
      
      std::cout << "Square wave at az=" << azimuthDeg << "°, el=" << elevationDeg 
                << "°, dist=" << distanceVal << "m, size=" << sizeVal
                << ", gain=" << gainVal << ", amplitude=" << amplitude 
                << " (attenuation=" << distanceAttenuation << ")" << std::endl;
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
  Mesh mesh;
  rnd::Random<> randomGenerator;
  DynamicScene scene;

  void onInit() override {
    auto speakers = AlloSphereSpeakerLayoutCompensated();    // stereo is for StereoSpeakerLayout()
    scene.setSpatializer<SpatializerType>(speakers);
    
    // We're using manual distance attenuation in the agents,
    // so we'll disable the automatic attenuation
    scene.distanceAttenuation().law(ATTEN_NONE);
    
    addDodecahedron(mesh);
    scene.setDefaultUserData(&mesh);
    scene.prepare(audioIO());
    
    std::cout << "Enhanced Distance Simulation with Independent Gain Control:" << std::endl;
    std::cout << "  1. Strong inverse square law attenuation (manual implementation)" << std::endl;
    std::cout << "  2. Enhanced air absorption filtering" << std::endl;
    std::cout << "  3. Independent gain control" << std::endl;
    std::cout << std::endl;
    std::cout << "Press SPACE to trigger sound objects at different positions:" << std::endl;
    std::cout << "  Sine wave: right side (90°, 30°, 8m, gain=1.0)" << std::endl;
    std::cout << "  Square wave: center front (0°, 0°, 3m, gain=0.7)" << std::endl;
    std::cout << "  Pink noise: left side (-90°, 30°, 1m, gain=0.5)" << std::endl;
  }

  void onDraw(Graphics &g) override {
    g.clear();
    scene.listenerPose(nav());
    scene.render(g);
  }

  void onSound(AudioIOData &io) override {
    scene.render(io);
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {
      float fps = graphicsDomain() ? graphicsDomain()->fps() : 60.0f;

      // Create objects at very different distances to make the effect obvious
      
      // Sine - right side, FAR, full gain
      MyAgent *sine = scene.getVoice<MyAgent>();
      if (sine) {
        sine->set(90.0f, 30.0f, 8.0f, 1.0f, 440.0f, 1.0f, fps * 4.0f);
        scene.triggerOn(sine);
      }

      // Square - center, MEDIUM, reduced gain
      SquareAgent *square = scene.getVoice<SquareAgent>();
      if (square) {
        square->set(0.0f, 0.0f, 3.0f, 1.0f, 330.0f, 0.7f, fps * 4.0f);
        scene.triggerOn(square);
      }
  
      // Pink - left side, CLOSE, lowest gain
      PinkAgent *pink = scene.getVoice<PinkAgent>();
      if (pink) {
        pink->set(-90.0f, 30.0f, 1.0f, 1.0f, 0.5f, fps * 4.0f);
        scene.triggerOn(pink);
      }
    }
    return true;
  }
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.configureAudio(44100, 256, 60, 0);   // for stereo, app.configureAudio(44100, 256, 2, 0); 60 for allosphere
  app.start();
  return 0;
}