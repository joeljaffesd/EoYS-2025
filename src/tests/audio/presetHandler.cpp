// Single macro to switch between desktop and Allosphere configurations
#define DESKTOP

#ifdef DESKTOP
  // Desktop configuration
  #define SAMPLE_RATE 48000
  #define AUDIO_CONFIG SAMPLE_RATE, 128, 2, 8
  #define SPATIALIZER_TYPE al::AmbisonicsSpatializer
  #define SPEAKER_LAYOUT al::StereoSpeakerLayout()
#else
  // Allosphere configuration
  #define SAMPLE_RATE 44100
  #define AUDIO_CONFIG SAMPLE_RATE, 256, 60, 8
  #define SPATIALIZER_TYPE al::Dbap
  #define SPEAKER_LAYOUT al::AlloSphereSpeakerLayoutCompensated()
#endif


#include "al/app/al_DistributedApp.hpp"
#include "src/graphics/videoToSphereCV.hpp"
#include "src/graphics/imageToSphere.hpp"
#include "src/graphics/assetEngine.hpp"
#include "src/graphics/shaderEngine.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "src/audio/audioManager.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "al/sound/al_Dbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"
#include "al/ui/al_PresetHandler.hpp"

// Gamma ig for now
#include "Gamma/SamplePlayer.h"

class Main : public al::DistributedApp {
public:
  int activeVoiceId;
  AudioManager<ChannelStrip> mManager;
  al::ParameterBool mAudioMode {"mAudioMode", "", false};
  al::ParameterBool mMute {"mMute", "", false};
  gam::SamplePlayer<float, gam::ipl::Cubic, gam::phsInc::Loop> player;

  void onInit() override {

    al::imguiInit();

    mManager.scene()->verbose(true);
    mManager.scene()->registerSynthClass<ChannelStrip>();
    mManager.scene()->registerSynthClass<ImageSphereLoader>();
    mManager.scene()->registerSynthClass<AssetEngine>();
    mManager.scene()->registerSynthClass<ShaderEngine>();
    mManager.scene()->registerSynthClass<VideoSphereLoaderCV>();
    this->registerDynamicScene(*mManager.scene());

    player.load("../assets/wavFiles/bass.wav");

    // Add sound agents
    mManager.addAgent("Bass");
    mManager.agents()->at(0)->mInputChannel = 0;

    // todo make this not suck
    mManager.agents()->at(0)->set(0.0, 90.0, 7.5, 1.0);
    mManager.initPresetHandlers();
    mManager.recallPresets();

    // TODO: encapsulate this in a function
    auto speakers = SPEAKER_LAYOUT; 
    mManager.scene()->setSpatializer<SPATIALIZER_TYPE>(speakers);
    mManager.scene()->distanceAttenuation().law(al::ATTEN_NONE);

    // prepare audio engine
    mManager.prepare(audioIO());

    // Set camera position and orientation
    if (isPrimary()) {
      nav().pos(al::Vec3d(35, 0.000000, 49));
      nav().quat(al::Quatd(1.0, 0.000000, 0.325568, 0.000000));
    }
    
  }

  void onSound(al::AudioIOData& io) override {
    if (isPrimary()) {
      // for now... write audio files to input to simulate audio input
      for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
        float input = player();
        io.inW(0, sample) = input;
      }

      if (!mMute) {
        mManager.processAudio(io);
      }
    }
  }

  void onAnimate(double dt) override {
    mManager.update(dt);
  }

  bool onKeyDown(const al::Keyboard& k) override {
  if (isPrimary()) {
    switch (k.key()) {
      case 'm':
        mMute = !mMute;
        break;
      case 'g':
        mAudioMode = !mAudioMode;
        break;
      case ' ':
        mManager.storePresets();
        break;
      default:
        break;  
      }
    }
    return true;
  }

  void onDraw(al::Graphics& g) override {
    g.lens().eyeSep(0.0); // disable stereo rendering
    g.clear(0);
    mManager.draw(g);
    if (isPrimary()) {
      if (mAudioMode) {
        mManager.drawGUI(g); // disallows access to other GUIs
      }
    }
  }

  // Mouse event handling
  bool onMouseMove(const al::Mouse& m) override {
    mManager.onMouseMove(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDown(const al::Mouse& m) override {
    mManager.onMouseDown(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDrag(const al::Mouse& m) override {
    mManager.onMouseDrag(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseUp(const al::Mouse& m) override {
    mManager.onMouseUp(graphics(), m, width(), height());
    return true;
  }
};

int main() {
  Main app;
  app.title("Preset Handler");
  app.configureAudio(AUDIO_CONFIG);
  app.start();
  return 0;
}