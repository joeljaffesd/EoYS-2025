#ifndef EOYS_ASSET_ENGINE_HPP
#define EOYS_ASSET_ENGINE_HPP

// Al includes
#include "al/app/al_App.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include "al/graphics/al_Image.hpp"

// std includes 
#include <vector>
#include <algorithm>

#include "graphicsVoice.hpp"

class AssetEngine : public GraphicsVoice {
private:
  al::Scene* ascene{nullptr};
  al::Vec3f scene_min, scene_max, scene_center;
  al::Texture tex;
  std::vector<al::Mesh> meshes;
  float a = 0.f, b = 0.f, c = 0.f;
  al::ParameterBool assetShow{"assetShow", "", true};
  al::Parameter scale{"scale", "", 1.f, 0.f, 10.f};
  al::ParameterBool rotate{"Rotate", "", true}; // Toggle rotation
  al::ControlGUI gui;
  al::ParameterBundle mParams{"AssetEngine"};

public:

  al::ParameterBundle& params() {
    return mParams;
  }

  void init(bool isReplica = false) override {
    this->GraphicsVoice::init(isReplica); // call base class init
    this->loadAsset("../assets/3dModels/eye/eye.obj",
                    "../assets/3dModels/eye/eye.png");
    gui << rotate << scale << assetShow;
    mParams << rotate << scale << assetShow;
    // this->registerParameters(rotate, scale, assetShow);

    if (ImGui::GetCurrentContext() == nullptr) {
      al::imguiInit();
    }
  }

  void loadAsset(std::string objPath, std::string imagePath) {
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

  void update(double dt = 0) override {
    if (this->isReplica) { return; }
    // Enable or disable rotation based on the GUI toggle
    if (!this->rotate) {
      this->a = 0.0f; // Stop rotation
    }
  }

  void onProcess(al::Graphics& g) override {

    if (!assetShow) {
      gui.draw(g); // draw gui regardless
    } else {
      //al::gl::depthTesting(true);
      g.lighting(true);
      g.pushMatrix();

      // animate rotation
      g.rotate(a, b, c, 0.f);
      a -= 0.2f;
      b += 0.2f;
      c += 0.2f;

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

      gui.draw(g); // draw gui regardless
    }
  }

  ~AssetEngine() {
    gui.cleanup();
  }
};

#endif // EOYS_ASSET_ENGINE_HPP