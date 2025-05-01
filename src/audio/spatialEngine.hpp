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

#include "../../Gimmel/include/filter.hpp"

#ifndef SPATIALIZER_TYPE
#define SPATIALIZER_TYPE AmbisonicsSpatializer // for Ambisonics
// #define SpatializerType Lbap // for playback in Allosphere
#endif

#ifndef SAMPLERATE
#define SAMPLERATE 44100
#endif

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
Vec3f sphericalToCartesian(float azimuthDeg, float elevationDeg, float radius) {
  // Convert azimuth and elevation from degrees to radians
  constexpr float degToRad = M_PI / 180.0f; // constexpr to increase efficiency 
  float azimuthRad = azimuthDeg * degToRad;
  float elevationRad = elevationDeg * degToRad;
  
  // Calculate the Cartesian coordinates using AlloSphere convention
  float cosElevRad = cos(elevationRad);
  float x = radius * cosElevRad * sin(azimuthRad);
  float y = radius * sin(elevationRad);
  float z = -radius * cosElevRad * cos(azimuthRad);  // Right-handed system flip
  
  return Vec3f(x, y, z);
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
void cartesianToSpherical(const Vec3f& cartesian, float& azimuthDeg, float& elevationDeg, float& radius) {
  float x = cartesian.x;
  float y = cartesian.y;
  float z = cartesian.z;
  
  radius = sqrt(x*x + y*y + z*z);
  
  if (radius < 1e-6) {
    azimuthDeg = 0.f;
    elevationDeg = 0.f;
    return;
  }
  
  elevationDeg = asin(y / radius) * 180.0f / M_PI;
  azimuthDeg = atan2(x, -z) * 180.0f / M_PI;  // Note the negative z for AlloSphere convention
}

/**
 * @class SelectablePickable
 * @brief A class that extends PickableBB to add selection functionality.
 */
class SelectablePickable : public PickableBB {
  public:
    bool selected = false;
  
    // set selected on hit
    bool onEvent(PickEvent e, Hit h) override {
      bool handled = PickableBB::onEvent(e, h);
      if (e.type == Pick && h.hit) { selected = true; }
      return handled;
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
class SpatialAgent : public PositionedVoice {
public:
  float phase = 0.0f;
  float freq = 440.0f;
  float baseAmplitude = 0.2f;
  float amplitude = 0.2f;
  float gain = 1.0f;
  float distance = 1.0f;
  float size = 1.0f;
  VAOMesh mMesh;
  SelectablePickable mPickable;
  giml::OnePole<float> airFilter;

  Parameter mAzimuth{"Azimuth", "", 90.0, "", -180.0, 180.0};
  Parameter mElevation{"Elevation", "", 30.0, "", -90.0, 90.0};
  Parameter mDistance{"Distance", "", 8.0, "", 0.1, 20.0};
  Parameter mGain{"Gain", "", 1.0, "", 0.0, 2.0};
  ControlGUI mGui;

  SpatialAgent() {
    addSphere(mMesh, 0.3);
    mMesh.primitive(Mesh::LINE_STRIP);
    mMesh.update();
  }

  void init() {
    mGui.init(5, 5, false);
    mGui.setTitle("Sine Wave");
    mGui << mAzimuth << mElevation << mDistance << mGain;

    mPickable.set(mMesh);
    mPickable.pose = Pose(sphericalToCartesian(mAzimuth.get(), mElevation.get(), mDistance.get()));

    // Register parameter callbacks for GUI updates
    // mAzimuth.registerChangeCallback([this](float value) {
    //   if (!pickablesUpdatingParameters) {
    //     updatePickablePositions();
    //   }
    // });
    
    // mElevation.registerChangeCallback([this](float value) {
    //   if (!pickablesUpdatingParameters) {
    //     updatePickablePositions();
    //   }
    // });
    
    // mDistance.registerChangeCallback([this](float value) {
    //   if (!pickablesUpdatingParameters) {
    //     updatePickablePositions();
    //   }
    // });

  }

  void onProcess(AudioIOData &io) override {
    float phaseInc = 2.0f * M_PI * freq / io.framesPerSecond();
    while (io()) {
      float sample = amplitude * gain * rnd::uniformS();
      sample = airFilter.lpf(sample);
      io.out(0) += sample;
      phase += phaseInc;
      if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
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
    
    float distanceAttenuation = 1.0f / (distance * distance);
    distanceAttenuation = std::min(distanceAttenuation, 5.0f);
    amplitude = baseAmplitude * distanceAttenuation;

    float distCutoff = 20000.0f / (1.0f + distance * 0.8f);
    distCutoff = std::max(distCutoff, 300.0f);
    airFilter.setCutoff(distCutoff, SAMPLERATE);
  }

  void onTriggerOn() override {
    phase = 0.0f;
  }
};