#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/io/al_File.hpp"
#include "al/graphics/al_Graphics.hpp"

using namespace al;

struct ImageSphereLoader {
  Mesh mMesh;
  File file = File::currentPath() + "../assets/imgWrap.png";
  Image image;
  ParameterBool imageShow{"imageShow", "", true};
  Parameter sphereRadius = {"sphereRadius", "", 3.f, 0.f, 10.f}; // You can adjust this value to make the sphere larger
  Parameter pointSize = {"pointSize", "", 10.f, 0.f, 100.f}; // Point size for the mesh

  void init() {

    image = Image(file.path());
    if(!image.loaded()) {
      std::cerr << "Failed to load image: " << file.path() << std::endl;
      return;
    }
    
    mMesh.primitive(Mesh::POINTS);
    for (int j = 0; j < image.height(); j++) { 
      for (int i = 0; i < image.width(); i++) {
        auto pixel = image.at(i, j);
        mMesh.vertex((1.0 * i) / image.width(), (1.0 * (image.height() - 1 - j)) / image.height(), 0); // Flip the y-axis
        mMesh.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);
      }
    }
  }

  void update() {
    this->createSphere();
  }

  void draw(Graphics& g) {
    if (!imageShow) return;
    g.meshColor();
    g.pointSize(pointSize); // play with 
    g.draw(mMesh);
  }

  void createSphere() {
    // Reset targetMesh
    mMesh.reset();
    // Map image onto a sphere
    for (int j = 0; j < image.height(); j++) {
      for (int i = 0; i < image.width(); i++) {
      auto pixel = image.at(i, j);

      // Convert 2D image pixel to spherical coordinates
      float longitude = (float)i / image.width() * M_2PI - M_PI;  // Map to [-PI, PI]
      float latitude = (float)(image.height() - 1 - j) / image.height() * M_PI - M_PI_2; // Flip the y-axis and map to [-PI/2, PI/2]

      // Convert spherical coordinates to Cartesian coordinates
      // Multiply by sphereRadius
      float x = cos(latitude) * cos(longitude) * sphereRadius;
      float y = sin(latitude) * sphereRadius;
      float z = cos(latitude) * sin(longitude) * sphereRadius;

      mMesh.vertex(x, y, z);
      mMesh.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);
      }
    }

    // Center the mesh on the origin
    Vec3f center(0.0f, 0.0f, 0.0f);
    for (auto& vertex : mMesh.vertices()) {
      center += vertex;
    }
    center /= mMesh.vertices().size();
    for (auto& vertex : mMesh.vertices()) {
      vertex -= center;
    }

    //std::cout << "Displaying Image Wrapped Around a Sphere." << std::endl;
  }

};