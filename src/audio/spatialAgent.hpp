#ifndef EOYS_SPATIAL_AGENT
#define EOYS_SPATIAL_AGENT

#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_PickableManager.hpp"
#include "al/graphics/al_Font.hpp"

#include "../../Gimmel/include/filter.hpp"

#include "tabbedGUI.hpp"

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
al::Vec3f sphericalToCartesian(float azimuthDeg, 
                               float elevationDeg, 
                               float radius) {
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
 * @brief A spatialized audio agent with logic for object-based sound spatialization. 
 * Includes a mesh for visualizing position, and a GUI.
 */
class SpatialAgent : public al::PositionedVoice {
public:
  float distance = 1.0f;
  float size = 1.0f;
  unsigned int sampleRate;
  al::HSV color = al::HSV(1.0);
  PickableMesh mPickableMesh;
  al::FontRenderer mFontRenderer;
  std::string mName;

  al::Parameter mAzimuth{ "Azimuth", "", 0.0, "", -180.0, 180.0 };
  al::Parameter mElevation{ "Elevation", "", 0.0, "", -90.0, 90.0 };
  al::Parameter mDistance{ "Distance", "", 1.0, "", 0.1, 20.0 };
  al::ParameterBundle mSpatializationParams{ "Spatialization" };
  TabbedGUI mGui;

  SpatialAgent(const char channelName[] = "No Name") {
    this->color = al::HSV(al::rnd::uniform(), 1.0, 1.0);
    mFontRenderer.load(al::Font::defaultFont().c_str(), 64, 2048);
    mName = channelName;
    this->registerParameters(mAzimuth, mElevation, mDistance);
  }

  void setName(const char name[]) {
    mFontRenderer.write(name);
    mGui.setTitle(name);
    mName = name;
  }

  std::string& name() {
    return mName;
  }

  void init() {
    mGui.init(5, 5, false);
    mSpatializationParams << mAzimuth << mElevation << mDistance;
    mGui << mSpatializationParams;

    mPickableMesh.pose = al::Pose(sphericalToCartesian(mAzimuth.get(), 
                                  mElevation.get(), 
                                  mDistance.get()));

  }

  void onProcess(al::AudioIOData& io) override {
    for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
      float output = al::rnd::uniformS();
      io.out(0) = output;
    }
  }

  void onProcess(al::Graphics& g) override {
    if (!mIsReplica) {
      g.color(this->color);
      g.draw(mPickableMesh);
      mFontRenderer.renderAt(g, al::Vec3d(0.0));

      // TODO: PR this into FontRenderer
      g.blending(false); // to undo blending call from FontRenderer 
    }
  }

  void set(float azimuthDeg, float elevationDeg, float distanceVal, float sizeVal) {
    al::Vec3f position = sphericalToCartesian(azimuthDeg, elevationDeg, distanceVal);
    this->setPose(al::Pose(position));
    mPickableMesh.pose = al::Pose(position);
    distance = distanceVal;
    size = sizeVal;
  }
};

#endif // EOYS_SPATIAL_AGENT