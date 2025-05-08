// file for testing loading in shaders onto sphere

#include "al/app/al_DistributedApp.hpp"
#include "src/graphics/shaderToSphere.hpp"
#include "src/graphics/audioReactor.hpp" 

#include "../../../Gimmel/include/filter.hpp"
#include "src/graphics/vfxUtility.hpp"
#include "src/graphics/vfxMain.hpp"

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
  float centroid = 0.0f;
  float smoothCent;
  float rms = 0.0f;
  int frame = 0;
  float newOnset = 0.0f;
  float smoothFlux;
  SmoothedValue smooth;
  SmoothedValue smooth2;

  // RippleEffect ripple;;
  // VertexEffectChain effectChain;

  giml::OnePole<float> mOnePole;
  giml::OnePole<float> mOnePoleCent;


    void onCreate() override {
        dynListen.setOnsetThresh(0.035);
        dynListen.setSilenceThresh(0.1);
        // *** SET SHADER PATH HERE *** //
        // for shaders folder 1: 
        shaderSphere.setShaders("../src/shaders/Moving-shaders/moving.vert", "../src/shaders/Reactive-shaders/fractalTest.frag");
        // for shader folder 2:
        //shaderSphere.setShaders("../src/shaders/Shader-2/shaderToyDefault.vert", "../src/shaders/Shader-2/shader2.frag");
        // *** END SET SHADER PATH *** //
        shaderSphere.setSphere(15.0f, 1000);

        // ripple.setParams(3.0, 2.0, 4.0, 'y');
        // effectChain.pushBack(&ripple);

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

        //old logic ->
        // if (newOnset == 0.0){
        // newOnset = 1.0f;
        // }
        // else{
        //   newOnset = 0.0f;
        // }
        //new logic ->
        newOnset += 0.1f;
  
      }
      // else if (dynListen.detectOnset() && newOnset == 0.0f) {
      //   std::cout << "New Onset detected!" << std::endl;
      //   newOnset = 1.0f;
      // }
      // else{
      //   newOnset = 0;
      //}
      if (++frame % 30 == 0) {
     // basic flux implemented 
      flux = specListen.getFlux();
      //  basic centroid implemented 
      centroid = specListen.getCent();

      //print every 30 times the loop is run
      //std::cout << "Flux: " << flux << ", Centroid: " << centroid << ", RMS: " << rms << std::endl;

    }
  }


    void onAnimate(double dt) override {
        t += dt;
        //smooth.setSmoothingFactor(0.1);
        mOnePoleCent.setCutoff(15000, 60);
        smoothCent = mOnePoleCent.lpf(centroid);
        //smoothCent = smooth.smooth(centroid);
       std::cout << "flux" << smoothFlux <<  std::endl;

       //smooth.setSmoothingFactor(0.05);
      //smoothFlux = smooth.smooth(flux);

      mOnePole.setCutoff(15000, 60);
      smoothFlux = mOnePole.lpf(flux);

      // effectChain.process(shaderSphere.shadedMesh.mesh, t);


    }

    void onDraw(al::Graphics& g) override {
        g.clear(0);
        shaderSphere.setMatrices(view().viewMatrix(), view().projMatrix(width(), height()));
        shaderSphere.shadedMesh.shader.use();


        shaderSphere.setUniformFloat("u_time", (float)t);
        shaderSphere.setUniformFloat("onset", newOnset);
        shaderSphere.setUniformFloat("cent", (smoothCent));
        shaderSphere.setUniformFloat("flux", smoothFlux);
        std::cout << "flux:" << smoothFlux << "cent:" << smoothCent << std::endl;
      


        shaderSphere.draw(g);
    }
};

int main() {
    MyApp app;
    app.start();
}
