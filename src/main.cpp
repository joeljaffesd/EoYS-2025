#define NAM_SAMPLE_FLOAT
#include "al/app/al_DistributedApp.hpp"
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"
#include "../NeuralAmpModelerCore/NAM/all.h"
#include "sphereScope.hpp"

struct State {
  static const int dataSize = 48000;
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

public:
  void onInit() override {
    auto cuttleboneDomain = al::CuttleboneStateSimulationDomain<State>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }
    if (isPrimary()) { // load NAM model on primary
      const char* mPath = "../../resources/MarshallModel.nam";
      mModel = nam::get_dsp(mPath);
      mModel->ResetAndPrewarm(this->audioIO().framesPerSecond(), 1);
      mScope.init(audioIO().framesPerSecond()); // init scope
    } else {
      mScope.init(state().dataSize);
    }
  }

  void onSound(al::AudioIOData& io) override {
    if (isPrimary()) { // only do audio for primary
      for (int sample = 0; sample < io.framesPerBuffer(); sample++) {
        // calculate output from input
        float input = io.in(0, sample);
        float output = 0.f;
        mModel->process(&input, &output, 1);

        // write output to scope
        mScope.writeSample(output);

        // write output to all output channels
        for (int channel = 0; channel < io.channelsOut(); channel++) {
          io.out(channel, sample) = output;
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
    mMainApp.configureAudio(48000, 128, 2, 1);
  }

  mMainApp.start();

  return 0;
}