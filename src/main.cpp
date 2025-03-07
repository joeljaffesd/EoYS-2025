#include "al/app/al_DistributedApp.hpp"
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"

#include "../Gimmel/include/gimmel.hpp"

#define NAM_SAMPLE_FLOAT
#include "../microNam/NAM/all.h"
#include "../resources/MarshallModel.h"

#include "sphereScope.hpp"

#define SAMPLE_RATE 48000

struct State {
  static const int dataSize = SAMPLE_RATE;
  al::Vec3f data[dataSize]; // assuming sample rate 48000

  void pushMesh(al::Mesh& m) {
    for (int i = 0; i < dataSize; i++) {
      if (i < m.vertices().size()) { // safety
        data[i] = m.vertices()[i];
      }
    }
  }

  void pullMesh(al::Mesh& m) {
    for (int i = 0; i < dataSize; i++) {
      if (i < m.vertices().size()) { // safety
        m.vertices()[i] = data[i];
      }
    }
  }

};

class MainApp : public al::DistributedAppWithState<State> {
private:
  std::unique_ptr<nam::DSP> mModel; 
  SphereScope mScope;

  // declare fx
  giml::Detune<float> detuneL{ SAMPLE_RATE };  
  giml::Detune<float> detuneR{ SAMPLE_RATE };
  giml::Delay<float> longDelay{ SAMPLE_RATE, 1000 }; 
  giml::Delay<float> shortDelay{ SAMPLE_RATE, 1000 };

public:
  void onInit() override {
    auto cuttleboneDomain = al::CuttleboneStateSimulationDomain<State>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }
    if (isPrimary()) { // load NAM model on primary
      mModel = nam::get_dsp(MarshallModel);
      mScope.init(audioIO().framesPerSecond()); // init scope
    } else {
      mScope.init(state().dataSize);
    }

    // init giml fx
    detuneL.enable();
    detuneR.enable();
    longDelay.enable();
    shortDelay.enable();
    detuneL.setPitchRatio(0.993);
    detuneR.setPitchRatio(1.007);
    longDelay.setDelayTime(798);
    longDelay.setFeedback(0.20);
    longDelay.setBlend(1.0);
    longDelay.setDamping(0.7);
    shortDelay.setDelayTime(398);
    shortDelay.setFeedback(0.30);
    shortDelay.setBlend(1.0);
    shortDelay.setDamping(0.7);
  }

  void onSound(al::AudioIOData& io) override {
    if (isPrimary()) { // only do audio for primary
      mModel->process(const_cast<float*>(io.inBuffer()), io.outBuffer(), io.framesPerBuffer());
      mModel->finalize_(io.framesPerBuffer());
      for (int sample = 0; sample < io.framesPerBuffer(); sample++) {
        // calculate output from input
        float dry = io.out(0, sample);

        // add fx
        float outL = dry + (0.31 * longDelay.processSample(detuneL.processSample(dry)));
        float outR = dry + (0.31 * shortDelay.processSample(detuneR.processSample(dry)));

        // write output to scope
        mScope.writeSample(outL);

        // write output to all output channels
        for (int channel = 0; channel < io.channelsOut(); channel++) {
          if (channel % 2 == 0) {
            io.out(channel, sample) = outL;
          } else {
            io.out(channel, sample) = outR;
          }
        }
      }
    }  
  }

  void onAnimate(double dt) override {
    if (isPrimary()) {
      mScope.update();
      state().pushMesh(mScope);
    } else {
      state().pullMesh(mScope);
    }
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0);
    g.meshColor();
    g.draw(mScope);
  }

};

int main() {
  MainApp mMainApp;

  if (mMainApp.isPrimary()) {
    mMainApp.configureAudio(SAMPLE_RATE, 128, 2, 1);
  }

  mMainApp.start();

  return 0;
}