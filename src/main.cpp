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

// Gamma ig for now
#include "Gamma/SamplePlayer.h"

class Main : public al::DistributedApp {
public:
  std::vector<al::PositionedVoice*> voices;
  int activeVoiceId;
  AudioManager<ChannelStrip> mManager;
  al::ParameterBool mAudioMode {"mAudioMode", "", false};
  al::ParameterBool mMute {"mMute", "", false};

  gam::SamplePlayer<float, gam::ipl::Cubic, gam::phsInc::Loop> player[8];
  std::vector<std::string> names;

  void onInit() override {

    player[0].load("../assets/wavFiles/vocals.wav");
    player[1].load("../assets/wavFiles/guitar.wav");
    player[2].load("../assets/wavFiles/bass.wav");
    player[3].load("../assets/wavFiles/kick.wav");
    player[4].load("../assets/wavFiles/snare.wav");
    player[5].load("../assets/wavFiles/floorTom.wav");
    player[6].load("../assets/wavFiles/midTom.wav");
    player[7].load("../assets/wavFiles/highTom.wav");

    names.push_back("Vocals");
    names.push_back("Guitar");
    names.push_back("Bass");
    names.push_back("Kick");
    names.push_back("Snare");
    names.push_back("Floor Tom");
    names.push_back("Mid Tom");
    names.push_back("High Tom");

    // TODO: encapsulate this in a function
    auto speakers = SPEAKER_LAYOUT; 
    mManager.scene()->setSpatializer<SPATIALIZER_TYPE>(speakers);
    mManager.scene()->distanceAttenuation().law(al::ATTEN_NONE);

    mManager.scene()->verbose(true);
    mManager.scene()->registerSynthClass<ChannelStrip>();
    mManager.scene()->registerSynthClass<ImageSphereLoader>();
    mManager.scene()->registerSynthClass<AssetEngine>();
    mManager.scene()->registerSynthClass<ShaderEngine>();
    mManager.scene()->registerSynthClass<VideoSphereLoaderCV>();
    this->registerDynamicScene(*mManager.scene());
    
    // Setup camera for 3D viewing
    nav().pos(0, 0, 0);  // Position the camera
    nav().faceToward(al::Vec3f(0, 0, 0));  // Face toward origin

    // Add sound agents
    for (unsigned i = 0; i < 8; i++) {
      mManager.addAgent(names[i].c_str());
      mManager.agents()->at(i)->mInputChannel = i;
    }

    // todo make this not suck
    mManager.agents()->at(0)->set(0.0, 90.0, 7.5, 1.0, SAMPLE_RATE);
    mManager.agents()->at(1)->set(0.0, 90.0, 5.0, 1.0, SAMPLE_RATE);
    mManager.agents()->at(2)->set(0.0, -90.0, 5.0, 1.0, SAMPLE_RATE);
    mManager.agents()->at(3)->set(0.0, -90.0, 1.0, 1.0, SAMPLE_RATE);
    mManager.agents()->at(4)->set(0.0, 90.0, 3.5, 1.0, SAMPLE_RATE);
    mManager.agents()->at(5)->set(0.0, 30.0, 8.0, 1.0, SAMPLE_RATE);    // 0 degrees
    mManager.agents()->at(6)->set(120.0, 30.0, 8.0, 1.0, SAMPLE_RATE);  // 120 degrees
    mManager.agents()->at(7)->set(240.0, 30.0, 8.0, 1.0, SAMPLE_RATE);  // 240 degrees

    // prepare audio engine
    mManager.prepare(audioIO());

    voices.push_back(mManager.scene()->getVoice<ImageSphereLoader>());
    voices.push_back(mManager.scene()->getVoice<AssetEngine>());
    voices.push_back(mManager.scene()->getVoice<ShaderEngine>());
    voices.push_back(mManager.scene()->getVoice<VideoSphereLoaderCV>());

    // Set camera position and orientation
    nav().pos(al::Vec3d(35, 0.000000, 49));
    nav().quat(al::Quatd(1.0, 0.000000, 0.325568, 0.000000));
    
  }

  void onSound(al::AudioIOData& io) override {
    // for now... write audio files to input to simulate audio input
    for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
      float input[8];
      for (auto index = 0; index < 8; index++) {
        input[index] = player[index]();
        io.inW(index, sample) = input[index];
      }
    }

    //mManager.scene()->render(io);
    if (!mMute) {
      mManager.processAudio(io);
    }
  }

  void onAnimate(double dt) override {
    // Update the video sphere loader with the elapsed time
    mManager.update(dt);
  }

  int phase = 0;
  bool onKeyDown(const al::Keyboard& k) override {
    if (k.key() == ' ') {
      mManager.scene()->triggerOff(activeVoiceId);
      switch (phase) {
        case 0: {
          activeVoiceId = mManager.scene()->triggerOn(voices[0]);
          phase++;
          break;
        }
        case 1: {
          activeVoiceId = mManager.scene()->triggerOn(voices[1]);
          phase++;
          break;
        }
        case 2: {
          activeVoiceId = mManager.scene()->triggerOn(voices[2]);
          phase++;
          break;
        }
        case 3: {
          activeVoiceId = mManager.scene()->triggerOn(voices[3]);
          phase++;
          break;
        }
        default:
          phase = 0;
          break;
      }
    }

    if (k.key() == 'm') {
      mMute = !mMute;
    }

    if (k.key() == 'a') {
      mAudioMode = !mAudioMode;
    }

    return true;
  }

  void onDraw(al::Graphics& g) override {
    g.lens().eyeSep(0.0); // disable stereo rendering
    g.clear(0);
    // mManager.scene()->render(g);
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
  app.title("Main");
  app.configureAudio(AUDIO_CONFIG);
  app.start();
  return 0;
}