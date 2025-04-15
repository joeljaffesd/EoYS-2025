
//#include "al/core.hpp"
#include "al/app/al_App.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
//#include "module/img/loadImage.hpp"
#include <algorithm>  // max
#include <cstdint>    // uint8_t
#include <vector>

using namespace al;
using namespace std;

struct AssetEngine {
  Scene *ascene{nullptr};
  Vec3f scene_min, scene_max, scene_center;
  //Texture tex;
  vector<Mesh> meshes;
  float a = 0.f, b = 0.f, c = 0.f;  // current rotation angle
  ParameterBool assetShow{"assetShow", "", true};
  Parameter scale{"scale", "", 1.f, 0.f, 10.f};

  void loadAssets() {
    std::string fileName = "../assets/rough-human-ouroboros.obj";
    ascene = Scene::import(fileName);
    if (!ascene) {
      printf("error reading %s\n", fileName.c_str());
      return;
    }
    ascene->getBounds(scene_min, scene_max);
    scene_center = (scene_min + scene_max) / 2.f;
    ascene->print();

    // extract meshes from scene
    meshes.resize(ascene->meshes());
    for (int i = 0; i < ascene->meshes(); i += 1) {
      ascene->mesh(i, meshes[i]);
    }
  }

  void draw(Graphics &g) {
    if (!assetShow) return;
    gl::depthTesting(true);
    g.lighting(true);
    g.pushMatrix();

    // rotate it around the y axis
    g.rotate(a, b, c, 0.f);
    a -= 0.2f;
    b += 0.2f;
    c += 0.2f;

    // scale the whole asset to fit into our view frustum
    float tmp = scene_max[0] - scene_min[0];
    tmp = std::max(scene_max[1] - scene_min[1], tmp);
    tmp = std::max(scene_max[2] - scene_min[2], tmp);
    tmp = 2.f / tmp;
    g.scale(tmp);
    g.scale(scale);

    // center the model
    g.translate(-scene_center);
    
    // draw all the meshes in the scene
    for (auto &m : meshes) {
      g.draw(m);
    }
    
    g.popMatrix();
    g.lighting(false);
  }
};