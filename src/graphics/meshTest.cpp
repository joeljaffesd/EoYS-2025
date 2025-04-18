#include "al/app/al_App.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "audioReactor.hpp"
#include "src/graphics/manualPulse.hpp"
#include "vfxMain.hpp"
#include <cmath>
#include <iostream>
#include <ostream>

using namespace al;

// this is a mesh useful for testing effects. the mesh is pretty much static
// until a time-based vertex effect is applied

class MyApp : public App {
public:
  // Mesh mesh;
  VAOMesh mesh;
  VAOMesh mesh2;

  SpectralListener specListen;
  float t = 0.0f;

  int gridSize = 20;
  float spacing = 0.1f;

  // example effect instantiation

  RippleEffect ripple;
  RippleEffect ripple2;
  OrbitEffect orbit;
  OrbitEffect orbit2;
  AutoPulseEffect autoPulse;
  ManualPulseEffect mainPulse;
  ScatterEffect scatter;
  VertexEffectChain effectChain; // chain constructed
  VertexEffectChain effectChain2;

  void onCreate() override {
    nav().pos(0, 0, 3);
    mesh.primitive(Mesh::POINTS);
    // mesh.primitive(Mesh::TRIANGLES);
    // mesh.primitive(Mesh::TRIANGLE_FAN);

    for (int x = -gridSize; x <= gridSize; ++x) {
      for (int y = -gridSize; y <= gridSize; ++y) {
        for (int z = -gridSize; z <= gridSize; ++z) {
          mesh.vertex(Vec3f(x * spacing, y * spacing, z * spacing

                            ));
          mesh2.vertex(Vec3f(x / 2 * spacing, y / 2 * spacing, z / 2 * spacing

                             ));
        }
      }
    }

    mesh.color(1, 1, 1);
    // mesh2.color(0.6, 1, 1);

    //// Basic template ////

    //// Set Base Mesh for pulsing ////
    autoPulse.setBaseMesh(mesh.vertices());
    mainPulse.setBaseMesh(mesh.vertices());

    //// Set Parameters ////
    ripple.setParams(1.0, 0.5, 4.0, 'y');
    orbit.setParams(1.0, 1.0, {0, 2, 1}, 0, -1, 1, 1);
    autoPulse.setParams(1.0, 1.0, 1);
    //// push effects to chain /////
    // effectChain.pushBack(&autoPulse);
    //   effectChain.pushBack(&orbit);
    effectChain.pushBack(&ripple);
    // effectChain.pushBack(&mainPulse);
    //  effectChain.pushBack(&scatter); // not working yet

    /////// END MESH 1 EFFECTS //////

    //// START MESH 2 EFFECTS/////
    // effectChain2.pushBack(&orbit2);
  }
  // just for debugging spectral listener
  void onSound(AudioIOData &io) override {
    while (io()) {
      float inSample = io.in(0); // assuming mono input from channel 0
      specListen.process(inSample);

      // For testing, you can just pass it through:
      float outSample = inSample;
      io.out(0) = outSample;
      io.out(1) = outSample;
    }
  }

  void onAnimate(double dt) override {
    t += dt;
    float flux = (specListen.getFlux() * 1000.f);

    std::cout << flux << std::endl;
    // std::cout << "Centroid: " << centroid << std::endl;
    //  if (dt == 10) {
    //    scatter.trigger();
    //  }
    //  autoPulse.setParams((flux * 10), flux / 10.f, 1);
    mainPulse.setParams(flux / 100.0f, flux);

    ripple.setParams(1.0, flux / 10, 4.0, 'y');

    effectChain.process(mesh, t); // run process function on chain
    // effectChain2.process(mesh2, t);
    // mesh.color();
  }

  void onDraw(Graphics &g) override {
    g.clear(0.1);
    g.pointSize(2.0);

    float centroid = specListen.getCent();
    g.color((centroid / 4000.f), 0.5, 1.0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    g.draw(mesh);
    g.draw(mesh2);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
};

int main() {
  MyApp app;
  app.configureAudio(44100, 256, 2, 1); // 2 out, 1 in
  app.start();
}
