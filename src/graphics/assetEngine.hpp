#ifndef EOYS_ASSET_ENGINE_HPP
#define EOYS_ASSET_ENGINE_HPP

// Al includes
#include "al/app/al_App.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include "al/graphics/al_Image.hpp"

// std includes 
#include <vector>
#include <algorithm>

class AssetEngine : public al::PositionedVoice {
private:
  al::Scene* ascene{nullptr};
  al::Vec3f scene_min, scene_max, scene_center;
  al::Texture tex;
  std::vector<al::Mesh> meshes;
  al::Parameter a = {"a", "", 0.f};
  al::Parameter b = {"b", "", 0.f};
  al::Parameter c = {"c", "", 0.f};
  al::ParameterBool assetShow{"assetShow", "", true};
  al::Parameter scale{"scale", "", 1.f, 0.f, 10.f};
  al::ParameterBool rotate{"Rotate", "", true}; // Toggle rotation
  al::ControlGUI gui;
  bool initFlag = true;

public:
  void init() override {
    // this->loadAsset(); // moved to flag 
    gui << rotate << scale << assetShow;
    this->registerParameters(rotate, scale, assetShow, a, b, c, mPose);
  }

  void loadAsset(std::string objPath = "../assets/3dModels/eye/eye.obj", 
                 std::string imagePath = "../assets/3dModels/eye/eye.png") {
    ascene = al::Scene::import(objPath);
    if (!ascene) {
      printf("error reading %s\n", objPath.c_str());
      return;
    }

    ascene->getBounds(scene_min, scene_max);
    scene_center = (scene_min + scene_max) / 2.f;
    ascene->print();

    // extract meshes
    meshes.resize(ascene->meshes());
    for (int i = 0; i < ascene->meshes(); ++i) {
      ascene->mesh(i, meshes[i]);
    }

    // load texture
    al::Image img;
    if (img.load(imagePath)) {
      tex.create2D(img.width(), img.height());
      tex.filter(al::Texture::LINEAR);
      tex.submit(img.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);
    } else {
      printf("Failed to load texture from %s\n", imagePath.c_str());
    }
  }

  void onProcess(al::Graphics& g) {

    if (initFlag) {
      this->loadAsset();
      initFlag = false;
    }

    if (!assetShow) {
      if (!mIsReplica) {
        //gui.draw(g); // draw gui regardless
      }
    } else {
      al::gl::depthTesting(true);
      g.lighting(true);
      g.pushMatrix();

      if (this->rotate) {
        // animate rotation
        // TODO remember where we left off when we toggle rotation
        g.rotate(a.toFloat(), b.toFloat(), c.toFloat(), 0.f);
        a = a - 0.2f;
        b = b + 0.2f;
        c = c + 0.2f;
      }

      // center and scale the model
      float tmp = std::max({scene_max[0] - scene_min[0], scene_max[1] - scene_min[1], scene_max[2] - scene_min[2]});
      tmp = 2.f / tmp;
      g.scale(tmp);
      g.scale(scale);
      g.translate(-scene_center);

      // bind and draw with texture
      tex.bind(0);
      g.texture();  // enables texture usage
      for (auto &m : meshes) {
        g.draw(m);
      }
      tex.unbind(0);

      g.popMatrix();
      g.lighting(false);

      if (!mIsReplica) {
        //gui.draw(g); // draw gui regardless
      }
    }
  }

  ~AssetEngine() {
    gui.cleanup();
  }
};

#endif // EOYS_ASSET_ENGINE_HPP