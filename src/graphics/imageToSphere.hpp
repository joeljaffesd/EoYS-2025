#ifndef EOYS_IMAGE_SPHERE
#define EOYS_IMAGE_SPHERE

#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"

struct ImageSphereLoader : public al::PositionedVoice {
  al::VAOMesh mMesh;
  al::Texture tex;
  al::ParameterBool imageShow{"imageShow", "", true};
  al::Parameter sphereRadius = { "sphereRadius", "", 3.f, 0.f,10.f}; 
  al::Parameter pointSize = {"pointSize", "", 10.f, 0.f, 100.f};
  bool initFlag = true;

  al::ParameterBool networkedInitFlag {"networkedInitFlag", "", true};
  al::ParameterString mImageFilePath {"mImageFilePath", "", "../assets/images/imgWrap.png"};

  al::ParameterBool rotate{"rotate", "", false};
  al::Parameter rotation{"rotation", "", 0.f, 0.f, 360.f};
  al::Parameter rotationSpeed{"rotationSpeed", "", 0.01f, 0.f, 1.f}; 

  ImageSphereLoader() {
    this->registerParameters(mPose, mImageFilePath, rotate, rotation, rotationSpeed);
    this->registerParameters(networkedInitFlag, imageShow, sphereRadius, pointSize);
    networkedInitFlag.registerChangeCallback([this](bool value) {
      this->initFlag = true;
      std::cout << "NetworkedInitFlag changed, setting initFlag to true" << std::endl;
    });    
  }

  void toggleRotation(bool value) {
    rotate.set(value);
    std::cout << "Rotation toggled to: " << (value ? "true" : "false") << std::endl;
  }

  void setRotationSpeed(float speed) {
    rotationSpeed.set(speed);
    std::cout << "Rotation speed set to: " << speed << std::endl;
  }

  void init() override {
    addTexSphere(mMesh, 15, 250, true);
    // this->loadImage(); // handle in draw w/ flag
  }

  void setImageFilePath(const std::string& imageFilePath) {
    mImageFilePath.set(imageFilePath);
    this->networkedInitFlag = !networkedInitFlag;
  }

  void loadImage(std::string imagePath = "../assets/images/imgWrap.png") {
    // load texture
    std::cout << "Loading image from: " << imagePath << std::endl;
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

  void onProcess(al::Graphics &g) {
    if (initFlag) {
      this->loadImage(mImageFilePath);
      initFlag = false;
    }
    // this may need to be changed to handle mesh manipulations and shader..
    // manipulations
    if (!imageShow) { return; }

    al::gl::depthTesting(true);
    //g.lighting(true);
    g.pushMatrix();

    if (rotate) {
      g.rotate(rotation.get(), 0, 1, 0);
      if (isPrimary()) {
        rotation = rotation + rotationSpeed; // Increment rotation
        if (rotation.get() > 360.f) {
          rotation.set(0.f); // Reset rotation after a full circle
        }        
      }
    }    
    

    tex.bind(0);
    g.texture();
    g.draw(mMesh);
    tex.unbind(0);

    g.popMatrix();
    //g.lighting(false);
  }
};

#endif // EOYS_IMAGE_SPHERE