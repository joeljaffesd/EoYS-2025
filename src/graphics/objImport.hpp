#include "al/app/al_App.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include "al/graphics/al_Image.hpp"
#include <vector>
#include <algorithm>

using namespace al;
using namespace std;

struct AssetEngine {
  Scene *ascene{nullptr};
  Vec3f scene_min, scene_max, scene_center;
  Texture tex;
  vector<Mesh> meshes;
  float a = 0.f, b = 0.f, c = 0.f;
  ParameterBool assetShow{"assetShow", "", true};
  Parameter scale{"scale", "", 1.f, 0.f, 10.f};

  void loadAssets() {
    //std::string fileName = "../assets/3dModels/sphereEye.obj";
    std::string fileName = "../assets/3dModels/daEyeLegacyExport.obj";
    // std::string fileName = "../assets/3dModels/skel-size-test-10.obj";
    ascene = Scene::import(fileName);
    if (!ascene) {
      printf("error reading %s\n", fileName.c_str());
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
    Image img;
    if (img.load("../assets/3dModels/bakeshiz.png")) {
      // tex.create2D(img.width(), img.height(), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
      // tex.submit(img.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);
      // tex.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
      // tex.filterMag(Texture::LINEAR);
      // tex.generateMipmap();

      tex.create2D(img.width(), img.height());
      tex.filter(Texture::LINEAR);
      tex.submit(img.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);
      
    } else {
      printf("Failed to load texture: bakeshiz.png\n");
    }
  }

  void draw(Graphics &g) {
    if (!assetShow) return;

    gl::depthTesting(true);
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
    // g.noTexture();  // ❌ Removed: not defined in al::Graphics

    g.popMatrix();
    g.lighting(false);
  }
};