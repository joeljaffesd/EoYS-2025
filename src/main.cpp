#include "al/app/al_DistributedApp.hpp"
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"

#include "../Gimmel/include/gimmel.hpp"

#include "../resources/MarshallModel.h"

#include "channelStrip.hpp"
#include "graphics/sphereScope.hpp"

#define SAMPLE_RATE 44100

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

// Add NAM compatibility to giml
namespace giml {
  template<typename T, typename Layer1, typename Layer2>
  class AmpModeler : public Effect<T>, public wavenet::RTWavenet<1, 1, Layer1, Layer2> {
  public:
    T processSample(const T& input) override {
      if (!this->enabled) { return input; }
      return this->model.forward(input);
    }
  };
}

class MainApp : public al::DistributedAppWithState<State> {
private:
  SphereScope mScope;
  ModelWeights mModelWeights;
  std::unique_ptr<giml::AmpModeler<float, Layer1, Layer2>> mAmpModeler{
    std::make_unique<giml::AmpModeler<float, Layer1, Layer2>>()
  };
  ChannelStrip mChannelStrip;

public:
  void onInit() override {
    auto cuttleboneDomain = al::CuttleboneStateSimulationDomain<State>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }
    if (isPrimary()) { // load NAM model on primary

      // mAmpModeler = std::make_unique<giml::AmpModeler<float, Layer1, Layer2>>(SAMPLE_RATE);
      mAmpModeler->toggle(true);
      mAmpModeler->loadModel(mModelWeights.weights);
      mChannelStrip.addEffect(std::move(mAmpModeler));

      auto detune = std::make_unique<giml::Detune<float>>(SAMPLE_RATE);
      detune->toggle(true);
      detune->setPitchRatio(0.993);
      detune->setBlend(0.5);
      mChannelStrip.addEffect(std::move(detune));
  
      auto delay = std::make_unique<giml::Delay<float>>(SAMPLE_RATE);
      delay->toggle(true);
      delay->setDelayTime(398);
      delay->setFeedback(0.30);
      delay->setBlend(0.24);
      mChannelStrip.addEffect(std::move(delay));

      mScope.init(audioIO().framesPerSecond()); // init scope
      mChannelStrip.init();
    } else {
      mScope.init(state().dataSize);
    }
  }

  void onSound(al::AudioIOData& io) override {
    if (isPrimary()) { // only do audio for primary
      //mChannelStrip.processAudio(io);
      for (int sample = 0; sample < io.framesPerBuffer(); sample++) {
        float input = io.in(0, sample);
        float output = mChannelStrip.processSample(input);
        for (int channel = 0; channel < io.channelsOut(); channel++) {
          io.out(channel, sample) = output;
        }
        // write output to scope
        mScope.writeSample(io.out(0, sample));
      }
    }  
  }

  void onAnimate(double dt) override {
    if (isPrimary()) {
      mScope.update();
      state().pushMesh(mScope);
      mChannelStrip.update();
    } else {
      state().pullMesh(mScope);
    }
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0);
    mChannelStrip.draw(g);
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