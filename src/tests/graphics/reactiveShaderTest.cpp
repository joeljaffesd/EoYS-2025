// file for testing loading in shaders onto sphere

#include "al/app/al_DistributedApp.hpp"
#include "src/graphics/shaderToSphere.hpp"
#include "src/graphics/audioReactor.hpp" 

#include "../../../Gimmel/include/filter.hpp"
#include "src/graphics/vfxUtility.hpp"
#include "src/graphics/vfxMain.hpp"
#include <iostream>

struct MyApp : al::DistributedApp {
  ShadedSphere shaderSphere;
  SpectralListener specListen;
  DynamicListener dynListen;

  al::Parameter now {"now", "", 0.f, 0.f, std::numeric_limits<float>::max()};
  al::Parameter flux {"flux", "", 0.01f, 0.f, 1.f};
  al::Parameter centroid = {"centroid", "", 1.f, 0.f, 20000.f};
  al::Parameter rms = {"rms", "", 0.f, 0.f, 1.f};
  al::Parameter onsetIncrement = {"onsetIncrement", "", 0.f, 0.f, 100.f};
  al::ParameterBundle mParams {"Uniforms"};

  FloatReporter fluxReporter;
  FloatReporter centroidReporter;
  FloatReporter rmsReporter;
  giml::OnePole<float> mOnePole;
  giml::OnePole<float> mOnePoleCent;

  void onInit() override {
    mParams << now << flux << centroid << rms << onsetIncrement;
    this->parameterServer().registerParameterBundle(mParams);
  }

  void onCreate() override {
    dynListen.setOnsetThresh(0.03);
    dynListen.setSilenceThresh(0.1);
    // *** SET SHADER PATH HERE *** //
    // for shaders folder 1: 
    shaderSphere.setShaders("../src/shaders/Moving-shaders/moving.vert", 
                            "../src/shaders/Reactive-shaders/fractalTest.frag");
    // for shader folder 2:
    //shaderSphere.setShaders("../src/shaders/Shader-2/shaderToyDefault.vert", "../src/shaders/Shader-2/shader2.frag");
    // *** END SET SHADER PATH *** //
    shaderSphere.setSphere(15.0f, 1000);
  }

  void onSound(al::AudioIOData &io) override {
    if (isPrimary()) {
      while (io()) {
        float in = io.in(0); // mono for now
        specListen.process(in);
        dynListen.process(in);

        centroidReporter.write(specListen.getCent());
        fluxReporter.write(specListen.getFlux());
        rmsReporter.write(dynListen.getRMS());

        //IMPORTANT - since listeners are running process at samplerate, 
        // call onset detection in one of the drawing threads - will still be accurate//
      }
    }
  }


  void onAnimate(double dt) override {
    if (isPrimary()) {

      now = now + float(dt);

      mOnePoleCent.setCutoff(15000, 60);
      centroid = mOnePoleCent.lpf(centroidReporter.reportValue());

      mOnePole.setCutoff(1000, 60);
      flux = mOnePole.lpf(fluxReporter.reportValue());

      if (dynListen.detectOnset()) {
        std::cout << "NEW ONSET" << std::endl;
        onsetIncrement = onsetIncrement + 0.1f;
      }

    }
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0);
    g.shader(shaderSphere.shader());

    shaderSphere.setUniformFloat("u_time", now);
    shaderSphere.setUniformFloat("onset", onsetIncrement);
    shaderSphere.setUniformFloat("cent", centroid);
    shaderSphere.setUniformFloat("flux", flux);
  
    shaderSphere.draw(g);
  }
};

int main() {
  MyApp app;
  app.start();
}