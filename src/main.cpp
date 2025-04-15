// AlloLib includes 
#include "al/app/al_DistributedApp.hpp"
#include "al/scene/al_DistributedScene.hpp"

// Gimmel/RTNeural includes
#include "../Gimmel/include/gimmel.hpp"
#include "../assets/MarshallModel.h"

// EoYS includes
#include "channelStrip.hpp"
#include "graphics/sphereScope.hpp"
#include "graphics/imageToSphere.hpp"
#include "graphics/objImport.hpp"

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

class GraphicsVoice : public al::PositionedVoice {
private:
  ImageSphereLoader mImageSphereLoader;
  AssetEngine mAssetEngine;
  SphereScope mScope;
  
public:

  virtual void init() override {
    mImageSphereLoader.init();
    mImageSphereLoader.createSphere();
    mAssetEngine.loadAssets();
    mScope.init(SAMPLE_RATE);
  }

  virtual void writeSample(float sample) {
    mScope.writeSample(sample);
  }

  virtual void update(double dt = 0) override {
    mScope.update();
  }

  virtual void onProcess(Graphics& g) override {
    mAssetEngine.draw(g); // Draw the 3D object
    g.meshColor();
    g.draw(mScope);
    mImageSphereLoader.draw(g); // Draw the sphere
  }
};

class MainApp : public al::DistributedApp {
private:
  ModelWeights mModelWeights;
  giml::AmpModeler<float, Layer1, Layer2> mAmpModeler;

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
  GraphicsVoice* mGraphicsVoice; // TODO: use a smart pointer
  al::DistributedScene mDistributedScene;

  //al::ControlGUI mControlGUI;

public:
  void onInit() override {

    //mControlGUI.init();

    mDistributedScene.registerSynthClass<GraphicsVoice>();
    registerDynamicScene(mDistributedScene);
    mDistributedScene.verbose(true);

    // only do audio stuff on primary
    if (isPrimary()) { 

      mChannelStrip.init();

      // mAmpModeler = std::make_unique<giml::AmpModeler<float, Layer1, Layer2>>(SAMPLE_RATE);
      mAmpModeler.toggle(true);
      mAmpModeler.loadModel(mModelWeights.weights);
      mChannelStrip.getEffectsLine().pushBack(&mAmpModeler);

      mDetune = std::make_unique<giml::Detune<float>>(SAMPLE_RATE);
      mDetune->toggle(mDetuneToggle);
      mDetune->setPitchRatio(mDetunePitchRatio);
      mDetune->setBlend(mDetuneBlend);
      mDetuneBundle << mDetuneToggle << mDetunePitchRatio << mDetuneBlend;
      mChannelStrip.addBundle(mDetuneBundle);
      mChannelStrip.getEffectsLine().pushBack(mDetune.get());
  
      mDelay = std::make_unique<giml::Delay<float>>(SAMPLE_RATE);
      mDelay->toggle(mDelayToggle);
      mDelay->setDelayTime(mDelayTime);
      mDelay->setFeedback(mDelayFeedback);
      mDelay->setBlend(mDelayBlend);
      mDelayBundle << mDelayToggle << mDelayTime << mDelayFeedback << mDelayBlend;
      mChannelStrip.addBundle(mDelayBundle);
      mChannelStrip.getEffectsLine().pushBack(mDelay.get());
    }
  }

  void onSound(al::AudioIOData& io) override {
    if (isPrimary()) { // only do audio for primary
      //mChannelStrip.processAudio(io);
      for (int sample = 0; sample < io.framesPerBuffer(); sample++) {
        float input = io.in(0, sample);
        float output = mChannelStrip.getEffectsLine().processSample(input);
        for (int channel = 0; channel < io.channelsOut(); channel++) {
          io.out(channel, sample) = output;
        }
        // write output to scope
        if (mGraphicsVoice != nullptr) {
          mGraphicsVoice->writeSample(output);
        }
      }
    }  
  }

  void onAnimate(double dt) override {
    if (isPrimary()) {
      mDetune->setPitchRatio(mDetunePitchRatio);
      mDetune->setBlend(mDetuneBlend);
      mDetune->toggle(mDetuneToggle);

      mDelay->setDelayTime(mDelayTime);
      mDelay->setFeedback(mDelayFeedback);
      mDelay->setBlend(mDelayBlend);
      mDelay->toggle(mDelayToggle);

      mDistributedScene.update();
      mChannelStrip.update();
    }
  }

  bool onKeyDown(const al::Keyboard& k) override {
    if (isPrimary()) {
      if (k.key() == ' ') {
        if (mGraphicsVoice == nullptr) {
          mGraphicsVoice = mDistributedScene.getVoice<GraphicsVoice>();
          mDistributedScene.triggerOn(mGraphicsVoice);
        }
      }
    }
    return true;
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0.1);
    mDistributedScene.render(g);
    if (isPrimary()) {
      mChannelStrip.draw(g);
    }
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