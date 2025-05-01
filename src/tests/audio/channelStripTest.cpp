// AlloLib includes 
#include "al/app/al_App.hpp"

// Gimmel/RTNeural includes
#include "../Gimmel/include/gimmel.hpp"
#include "../assets/MarshallModel.h"

// EoYS includes
#include "channelStrip.hpp"

#define SAMPLE_RATE 44100

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

class MainApp : public al::App {
private:
  ModelWeights mModelWeights;
  std::unique_ptr<giml::AmpModeler<float, Layer1, Layer2>> mAmpModeler;

  al::ParameterBool mDetuneToggle{"DetuneToggle", "", false};
  al::Parameter mDetunePitchRatio{"DetunePitchRatio", "", 0.995, 0.0, 1.0};
  al::Parameter mDetuneBlend{"DetuneBlend", "", 0.24, 0.0, 1.0};
  al::ParameterBundle mDetuneBundle{"Detune"};
  std::unique_ptr<giml::Detune<float>> mDetune;

  al::ParameterBool mDelayToggle{"DelayToggle", "", true};
  al::Parameter mDelayTime{"DelayTime", "", 398, 0, 1000};
  al::Parameter mDelayFeedback{"DelayFeedback", "", 0.30, 0.0, 1.0};
  al::Parameter mDelayBlend{"DelayBlend", "", 0.24, 0.0, 1.0};
  al::ParameterBundle mDelayBundle{"Delay"};
  std::unique_ptr<giml::Delay<float>> mDelay;

  ChannelStrip mChannelStrip;

public:
  void onInit() override {

    mChannelStrip.init();

    // todo - make amp modeler play nice as a std::unique_ptr
    mAmpModeler = std::make_unique<giml::AmpModeler<float, Layer1, Layer2>>();
    mAmpModeler->toggle(true);
    mAmpModeler->loadModel(mModelWeights.weights);
    mChannelStrip.addEffect(std::move(mAmpModeler));

    mDetune = std::make_unique<giml::Detune<float>>(SAMPLE_RATE);
    mDetune->toggle(mDetuneToggle);
    mDetune->setPitchRatio(mDetunePitchRatio);
    mDetune->setBlend(mDetuneBlend);
    mDetuneBundle << mDetuneToggle << mDetunePitchRatio << mDetuneBlend;
    mChannelStrip.addBundle(mDetuneBundle);
    mChannelStrip.addEffect(std::move(mDetune));

    mDelay = std::make_unique<giml::Delay<float>>(SAMPLE_RATE);
    mDelay->toggle(mDelayToggle);
    mDelay->setDelayTime(mDelayTime);
    mDelay->setFeedback(mDelayFeedback);
    mDelay->setBlend(mDelayBlend);
    mDelayBundle << mDelayToggle << mDelayTime << mDelayFeedback << mDelayBlend;
    mChannelStrip.addBundle(mDelayBundle);
    mChannelStrip.addEffect(std::move(mDelay));
  }

  void onSound(al::AudioIOData& io) override {
    for (int sample = 0; sample < io.framesPerBuffer(); sample++) {
      float input = float(impulse);
      float output = mChannelStrip.processSample(input);
      for (int channel = 0; channel < io.channelsOut(); channel++) {
        io.out(channel, sample) = output;
      }
      impulse = false; // reset impulse 
    }
  }

  // fire an impulse when space is pressed
  bool impulse = false;
  bool onKeyDown(const al::Keyboard& k) override {
    if (k.key() == ' ') {
      if (!impulse) {
        impulse = true;
      }
    }
    return true;
  }

  void onDraw(al::Graphics& g) override {
    g.lens().eyeSep(0.0); // disable stereo rendering
    g.clear(0.1);
    //mChannelStrip.draw(g); // hmm... seg faults
  }

};

int main() {
  MainApp mMainApp;

  mMainApp.configureAudio(SAMPLE_RATE, 128, 2, 1);

  mMainApp.start();
  return 0;
}