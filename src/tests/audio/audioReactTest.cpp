#include "al/app/al_App.hpp"
#include "src/graphics/audioReactor.hpp" 
#include <iostream>

//File for testing audio reactor implementations. Used to print values for debugging.

class MyApp : public al::App {
public:
  SpectralListener specListen;
  DynamicListener dynListen;
  float flux = 0.0f;
  float centroid = 0.0f;
  float rms = 0.0f;
  int frame = 0;

//   void onInit() {
//     dynListen.setOnsetThresh(0.01);
//   }
  void onCreate() override {
    dynListen.setOnsetThresh(0.035);
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
      }
      if (++frame % 30 == 0) {
     // basic flux implemented 
      flux = specListen.getFlux();
      //  basic centroid implemented 
      centroid = specListen.getCent();

      //print every 30 times the loop is run
      std::cout << "Flux: " << flux << ", Centroid: " << centroid << ", RMS: " << rms << std::endl;

    }
  }
};

//   void onCreate() override {
//   }
// };

int main() {
  MyApp app;
  app.configureAudio(44100, 512, 1, 1); 
  app.start();
}
