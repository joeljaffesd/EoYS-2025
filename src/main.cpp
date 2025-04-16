// AlloLib includes 
#include "al/app/al_DistributedApp.hpp"
#include "al/scene/al_DistributedScene.hpp"
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"

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
  al::ParameterBool showScope{"showScope", "", true};
  
public:

  al::ParameterBundle mBundle{"Graphics"};
  SphereScope mScope;

  virtual void init() override {
    mImageSphereLoader.init();
    mImageSphereLoader.createSphere();
    mAssetEngine.loadAssets();
    mScope.init(SAMPLE_RATE);
    mBundle << mImageSphereLoader.sphereRadius;
    mBundle << mImageSphereLoader.pointSize;
    mBundle << mAssetEngine.scale;
    mBundle << mImageSphereLoader.imageShow;
    mBundle << mAssetEngine.assetShow;
    mBundle << showScope;
  }

  virtual void writeSample(float sample) {
    mScope.writeSample(sample);
  }

  virtual void update(double dt = 0) override {
    mImageSphereLoader.update();
    mScope.update();
  }

  virtual void onProcess(Graphics& g) override {
    mAssetEngine.draw(g); // Draw the 3D object
    g.meshColor();
    if (showScope) {
      g.draw(mScope);
    }
    mImageSphereLoader.draw(g); // Draw the sphere
  }
};

struct State {
  bool imageShow = true, assetShow = true, showScope = true;
  float sphereRadius = 0.f, pointSize = 0.f, scale = 0.f;
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

public:
  void onInit() override {

    auto cuttleboneDomain = al::CuttleboneStateSimulationDomain<State>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }

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

    mDistributedScene.update();

    if (isPrimary()) {
      mDetune->setPitchRatio(mDetunePitchRatio);
      mDetune->setBlend(mDetuneBlend);
      mDetune->toggle(mDetuneToggle);

      mDelay->setDelayTime(mDelayTime);
      mDelay->setFeedback(mDelayFeedback);
      mDelay->setBlend(mDelayBlend);
      mDelay->toggle(mDelayToggle);

      mChannelStrip.update();

      if (mGraphicsVoice != nullptr) {
        state().sphereRadius = mGraphicsVoice->mBundle.parameters()[0]->toFloat();
        state().pointSize = mGraphicsVoice->mBundle.parameters()[1]->toFloat();
        state().scale = mGraphicsVoice->mBundle.parameters()[2]->toFloat();
        state().imageShow = mGraphicsVoice->mBundle.parameters()[3]->toFloat();
        state().assetShow = mGraphicsVoice->mBundle.parameters()[4]->toFloat();
        state().showScope = mGraphicsVoice->mBundle.parameters()[5]->toFloat();
        state().pushMesh(mGraphicsVoice->mScope);
      }

    } else {
      // update the graphics voice with the state
      auto temp = mDistributedScene.getActiveVoices();
      mGraphicsVoice = static_cast<GraphicsVoice*>(temp);
      if (mGraphicsVoice != nullptr) {
        mGraphicsVoice->mBundle.parameters()[0]->fromFloat(state().sphereRadius);
        mGraphicsVoice->mBundle.parameters()[1]->fromFloat(state().pointSize);
        mGraphicsVoice->mBundle.parameters()[2]->fromFloat(state().scale);
        mGraphicsVoice->mBundle.parameters()[3]->fromFloat(state().imageShow);
        mGraphicsVoice->mBundle.parameters()[4]->fromFloat(state().assetShow);
        mGraphicsVoice->mBundle.parameters()[5]->fromFloat(state().showScope);
        state().pullMesh(mGraphicsVoice->mScope);
      }
    }
  }

  bool onKeyDown(const al::Keyboard& k) override {
    if (isPrimary()) {
      if (k.key() == ' ') {
        if (mGraphicsVoice == nullptr) {
          mGraphicsVoice = mDistributedScene.getVoice<GraphicsVoice>();
          mDistributedScene.triggerOn(mGraphicsVoice);
          mChannelStrip.addBundle(mGraphicsVoice->mBundle);
        }
      }
    }
    return true;
  }

  void onDraw(al::Graphics& g) override {
    g.lens().eyeSep(0.0); // disable stereo rendering
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