// file for testing loading in shaders onto sphere

#include "al/app/al_App.hpp"
#include "src/graphics/shaderToSphere.hpp"
#include "src/graphics/audioReactor.hpp" 

//using namespace al;

struct MyApp : al::App {
    ShaderToSphere shaderSphere;
    double t = 0.0;
    SpectralListener specListen;
  DynamicListener dynListen;
  float flux = 0.0f;
  float centroid = 0.0f;
  float rms = 0.0f;
  int frame = 0;
  int newOnset = 0;

    void onCreate() override {
        dynListen.setOnsetThresh(0.035);
        dynListen.setSilenceThresh(0.1);
        // *** SET SHADER PATH HERE *** //
        // for shaders folder 1: 
        shaderSphere.setShaders("../src/shaders/Moving-shaders/moving.vert", "../src/shaders/Reactive-shaders/reactive1.frag");
        // for shader folder 2:
        //shaderSphere.setShaders("../src/shaders/Shader-2/shaderToyDefault.vert", "../src/shaders/Shader-2/shader2.frag");
        // *** END SET SHADER PATH *** //
        shaderSphere.setSphere(15.0f, 1000);

        nav().pos(0, 0, 6);
        navControl().active(true);
    }
    void onSound(al::AudioIOData &io) override {
    while (io()) {
      float in = io.in(0); // mono for now
      specListen.process(in);
      dynListen.process(in);
    }

    //increment frame in on sound
    // need to tweak values, "frames" don't currently correlate to actual visual frame rate. crude implementation for now
    

    // basic RMS implemented 
      rms = dynListen.getRMS();
      //std::cout << "Flux: " << flux << ", Centroid: " << centroid << ", RMS: " << rms << std::endl;
      // basic onset detection - 
      if (dynListen.detectOnset()) {
        std::cout << "New Onset detected!" << std::endl;
        newOnset = 0;
      }
      // else{
      //   newOnset = 0;
      //}
      if (++frame % 30 == 0) {
     // basic flux implemented 
      flux = specListen.getFlux();
      //  basic centroid implemented 
      centroid = specListen.getCent();

      //print every 30 times the loop is run
      std::cout << "Flux: " << flux << ", Centroid: " << centroid << ", RMS: " << rms << std::endl;

    }
  }


    void onAnimate(double dt) override {
        t += dt;
    }

    void onDraw(al::Graphics& g) override {
        g.clear(0);
        shaderSphere.setMatrices(view().viewMatrix(), view().projMatrix(width(), height()));
        //doing this to handle uniforms being stuck . didnt need for example 1
        shaderSphere.shadedMesh.shader.use();


        //shaderSphere.shadedMesh.use(); //not properly accessing this member
        //For shader 1:
        shaderSphere.setUniformFloat("u_time", (float)t);
        shaderSphere.setUniformFloat("onset", int(newOnset));
        //For shader 2:
        //shaderSphere.setUniformFloat("iTime", t);
        //shaderSphere.setUniformVec3f("iResolution", Vec3f(width(), (height()), 0.0f)); // this simulates values of a screen size. it gets passed into the vertex shader then converted to normalized spherical coords. should maybe change where this operation happens. 


        shaderSphere.draw(g);
    }
};

int main() {
    MyApp app;
    app.start();
}
