#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"

struct ImageSphereLoader {
  al::VAOMesh mMesh;
  al::Texture tex;
  al::ParameterBool imageShow{"imageShow", "", true};
  al::Parameter sphereRadius = { "sphereRadius", "", 3.f, 0.f,10.f}; 
  al::Parameter pointSize = {"pointSize", "", 10.f, 0.f, 100.f};

  void init() {
    addTexSphere(mMesh, 15, 250, true);
    this->loadImage();
  }

  void loadImage(std::string imagePath = "../assets/images/imgWrap.png") {
    // load texture
    al::Image image;
    if (image.load(imagePath)) {
      tex.create2D(image.width(), image.height());
      tex.filter(al::Texture::LINEAR);
      tex.submit(image.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);
    } else {
      printf("Failed to load texture from %s\n", imagePath.c_str());
    }

    for (int j = 0; j < image.height(); j++) {
      for (int i = 0; i < image.width(); i++) {
        // Get the pixel color at position (i, j)
        auto pixel = image.at(i, j);

        // Convert 2D image pixel to spherical coordinates
        float longitude =
            (float)i / image.width() * M_2PI - M_PI; // Map to [-PI, PI]
        float latitude =
            (float)(image.height() - 1 - j) / image.height() * M_PI -
            M_PI_2; // Flip the y-axis and map to [-PI/2, PI/2]

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
    al::Vec3f center(0.0f, 0.0f, 0.0f);
    for (auto& vertex : mMesh.vertices()) {
      center += vertex;
    }
    center /= mMesh.vertices().size();
    for (auto& vertex : mMesh.vertices()) {
      vertex -= center;
    }
    mMesh.update();

  } 

  void draw(al::Graphics &g) {
    // this may need to be changed to handle mesh manipulations and shader..
    // manipulations
    if (!imageShow) { return; }

    al::gl::depthTesting(true);
    //g.lighting(true);
    g.pushMatrix();

    tex.bind(0);
    g.texture();
    g.draw(mMesh);
    tex.unbind(0);

    g.popMatrix();
    //g.lighting(false);
  }
};