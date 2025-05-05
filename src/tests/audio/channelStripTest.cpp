#define SAMPLE_RATE 48000 // for lap/desktop computer
#ifndef SAMPLE_RATE
#define SAMPLE_RATE 44100 // for playback in Allosphere
#endif

#define AUDIO_CONFIG SAMPLE_RATE, 128, 2, 1 // for lap/desktop computer 
#ifndef AUDIO_CONFIG
#define AUDIO_CONFIG SAMPLE_RATE, 256, 60, 8 // for playback in Allosphere
#endif

#define SPATIALIZER_TYPE al::AmbisonicsSpatializer // for Ambisonics
#ifndef SPATIALIZER_TYPE
#define SPATIALIZER_TYPE al::Lbap // for playback in Allosphere
#endif

#define SPEAKER_LAYOUT al::StereoSpeakerLayout() // for lap/desktop computer 
#ifndef SPEAKER_LAYOUT
#define SPEAKER_LAYOUT al::AlloSphereSpeakerLayoutCompensated() // for playback in Allosphere
#endif

// AlloLib includes 
#include "al/app/al_DistributedApp.hpp"

// Gimmel/RTNeural includes
#include "../Gimmel/include/gimmel.hpp"
#include "../assets/MarshallModel.h"

// EoYS includes
#include "../../audio/channelStrip.hpp"
#include "../../audio/audioManager.hpp"

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

class ChannelStripTestApp : public al::DistributedApp {
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

  AudioManager mAudioManager;

public:
  void onInit() override {

    // TODO: encapsulate this in a function
    auto speakers = SPEAKER_LAYOUT; 
    mAudioManager.scene()->setSpatializer<SPATIALIZER_TYPE>(speakers);
    mAudioManager.scene()->distanceAttenuation().law(al::ATTEN_NONE);
    
    // Set fixed listener pose at origin (0,0,0)
    mAudioManager.setListenerPose(al::Pose(al::Vec3f(0, 0, 0)));

    // Add sound agents
    for (unsigned i = 0; i < 3; i++) {
      mAudioManager.addAgent();
    }

    // prepare audio engine
    mAudioManager.prepare(audioIO());

    // todo - make amp modeler play nice as a std::unique_ptr
    // mAmpModeler = std::make_unique<giml::AmpModeler<float, Layer1, Layer2>>();
    // mAmpModeler->toggle(true);
    // mAmpModeler->loadModel(mModelWeights.weights);
    // mChannelStrip.addEffect(std::move(mAmpModeler));

    // mDetune = std::make_unique<giml::Detune<float>>(SAMPLE_RATE);
    // mDetune->toggle(mDetuneToggle);
    // mDetune->setPitchRatio(mDetunePitchRatio);
    // mDetune->setBlend(mDetuneBlend);
    // mDetuneBundle << mDetuneToggle << mDetunePitchRatio << mDetuneBlend;
    // mAudioManager..addBundle(mDetuneBundle);
    // mChannelStrip.addEffect(std::move(mDetune));

    // mDelay = std::make_unique<giml::Delay<float>>(SAMPLE_RATE);
    // mDelay->toggle(mDelayToggle);
    // mDelay->setDelayTime(mDelayTime);
    // mDelay->setFeedback(mDelayFeedback);
    // mDelay->setBlend(mDelayBlend);
    // mDelayBundle << mDelayToggle << mDelayTime << mDelayFeedback << mDelayBlend;
    // mChannelStrip.addBundle(mDelayBundle);
    // mChannelStrip.addEffect(std::move(mDelay));
  }

  void onCreate() override {
    // Set up GUI windows
    al::imguiInit();
    
    // Set up camera - still movable but doesn't affect audio
    nav().pos(0, 0, 10);
    
    std::cout << "3D Sound Spatialization with GUI and Pickable Objects:" << std::endl;
    std::cout << "  1. Click on a sound source to show its control panel" << std::endl;
    std::cout << "  2. Click and drag objects to move them in space" << std::endl;
    std::cout << "  3. Camera can be moved with arrow keys (view only, doesn't affect audio)" << std::endl;
  }

  void onSound(al::AudioIOData& io) override {
    // for (int sample = 0; sample < io.framesPerBuffer(); sample++) {
    //   float input = float(impulse);
    //   float output = mChannelStrip.processSample(input);
    //   for (int channel = 0; channel < io.channelsOut(); channel++) {
    //     io.out(channel, sample) = output;
    //   }
    //   impulse = false; // reset impulse 
    // }
    mAudioManager.processAudio(io);
  }

  void onAnimate(double dt) override {
    mAudioManager.update();
  }

  void onDraw(al::Graphics& g) override {
    g.lens().eyeSep(0.0); // disable stereo rendering
    g.clear(0.1);
    al::gl::depthTesting(true);
    //mChannelStrip.draw(g); // hmm... seg faults
    mAudioManager.draw(g);
    mAudioManager.drawGUI(g);
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

  // Mouse event handling
  bool onMouseMove(const al::Mouse& m) override {
    mAudioManager.onMouseMove(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDown(const al::Mouse& m) override {
    mAudioManager.onMouseDown(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDrag(const al::Mouse& m) override {
    mAudioManager.onMouseDrag(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseUp(const al::Mouse& m) override {
    mAudioManager.onMouseUp(graphics(), m, width(), height());
    return true;
  }

};

int main() {
  ChannelStripTestApp app;
  app.title("Channel Strip Test");
  app.configureAudio(AUDIO_CONFIG);
  app.start();
  return 0;
}