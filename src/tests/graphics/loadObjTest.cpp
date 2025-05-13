
#include "al/app/al_App.hpp"

//#include "al/graphics/al_Asset.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Graphics.hpp"

#include "src/graphics/objParser.hpp"


// //might be using for parsing
// #include <fstream>
// #include <sstream>
// #include <iostream>
// #include <string>

using namespace al;


class MyApp : public App {
 public:
  al::Mesh bodyMesh; //switch to vao mesh
  objParser newObjParser;

  void onCreate() override {
    //nav().pos(Vec3d(0, 0, 8));  // Set the camera to view the scene
    //bodyMesh.primitive(Mesh:: POINTS);
    bodyMesh.primitive(Mesh:: TRIANGLE_FAN);
    //newObjParser.parse("/Users/lucian/Desktop/201B/allolib_playground/softlight-sphere/BaseMesh.obj", bodyMesh);
    newObjParser.parse("/Users/lucian/Desktop/EoYS-2025/assets/lotus-materials.obj", bodyMesh);


  }

  void onAnimate(double dt) override {

  }

  void onDraw(Graphics& g) override {
    g.clear(0);
    g.color(1.0);
    g.pointSize( 10.0);
    g.draw(bodyMesh);

    
  }

};

int main() {
  MyApp app;
  app.start();
  return 0;
}
