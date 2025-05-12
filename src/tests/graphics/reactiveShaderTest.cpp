// file for testing loading in shaders onto sphere

#include "al/app/al_DistributedApp.hpp"
#include "src/graphics/shaderToSphere.hpp"
#include "src/graphics/audioReactor.hpp" 

#include "../../../Gimmel/include/filter.hpp"
#include "src/graphics/vfxUtility.hpp"
#include "src/graphics/vfxMain.hpp"
#include <iostream>

/* TO DO :
* move var memory writing outside of audio callback - buffer approach?
* move printing and writing to draw
*
*/

struct MyApp : al::DistributedApp {
    ShaderToSphere shaderSphere;
    double t = 0.0;
    SpectralListener specListen;
  DynamicListener dynListen;
  float flux = 0.01f;
  float centroid = 1.0f;
  float rms = 0.0f;
  float onsetIncrement = 0.0f;
  FloatReporter fluxReporter;
  FloatReporter centroidReporter;
  FloatReporter rmsReporter;
  //BoolReporter onsetReporter;


  giml::OnePole<float> mOnePole;
  giml::OnePole<float> mOnePoleCent;


    void onCreate() override {
        dynListen.setOnsetThresh(0.03);
        dynListen.setSilenceThresh(0.1);
        // *** SET SHADER PATH HERE *** //
        // for shaders folder 1: 
        shaderSphere.setShaders("../src/shaders/Moving-shaders/moving.vert", "../src/shaders/Reactive-shaders/fractalTest.frag");
        // for shader folder 2:
        //shaderSphere.setShaders("../src/shaders/Shader-2/shaderToyDefault.vert", "../src/shaders/Shader-2/shader2.frag");
        // *** END SET SHADER PATH *** //
        shaderSphere.setSphere(15.0f, 1000);

        nav().pos(0, 0, 0);
        navControl().active(true);
    }
    void onSound(al::AudioIOData &io) override {
    while (io()) {
      float in = io.in(0); // mono for now
      specListen.process(in);
      dynListen.process(in);

      centroidReporter.write(specListen.getCent());
      fluxReporter.write(specListen.getFlux());
      rmsReporter.write(dynListen.getRMS());

      //IMPORTANT - since listeners are running process at samplerate, call onset detection in one of the drawing threads - will still be accurate//

    

    }
  }


    void onAnimate(double dt) override {

      if (dynListen.detectOnset() == true){
      std::cout << "NEW ONSET" << std::endl;
      onsetIncrement +=0.1;
      }

      t += dt;
    

      mOnePoleCent.setCutoff(15000, 60);
      centroid = mOnePoleCent.lpf(centroidReporter.reportValue());
      mOnePole.setCutoff(1000, 60);
      flux = mOnePole.lpf(fluxReporter.reportValue());

    }

    void onDraw(al::Graphics& g) override {
        g.clear(0);
        shaderSphere.setMatrices(view().viewMatrix(), view().projMatrix(width(), height()));
        shaderSphere.shadedMesh.shader.use();


        shaderSphere.setUniformFloat("u_time", t);
        shaderSphere.setUniformFloat("onset", onsetIncrement);
        shaderSphere.setUniformFloat("cent", centroid);
        shaderSphere.setUniformFloat("flux", flux);
        std::cout << "flux: " << flux << "cent: " << centroid << std::endl;
      


        shaderSphere.draw(g);
    }
};

int main() {
    MyApp app;
    app.start();
}
