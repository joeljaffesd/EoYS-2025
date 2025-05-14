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

// AlloLib includes 
#include "al/app/al_DistributedApp.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "al/sound/al_Dbap.hpp"

// Gimmel/RTNeural includes
#include "../Gimmel/include/gimmel.hpp"
#include "../assets/namModels/MarshallModel.h"

// EoYS includes
#include "../../audio/channelStrip.hpp"
#include "../../audio/audioManager.hpp"
#include "../../audio/ampModeler.hpp"

// Gamma ig for now
#include "Gamma/SamplePlayer.h"

class ChannelStripTestApp : public al::DistributedApp {
private:
  // Audio engine, handles spatialization and distribution over network
  AudioManager<ChannelStrip> mAudioManager;

  gam::SamplePlayer<float, gam::ipl::Cubic, gam::phsInc::Loop> player[8];
  std::vector<std::string> names;

  // reference sphere for audio source positions
  al::Mesh mSphereReference;

public:
  void onInit() override {

    this->nav().set(al::Nav(10.f));
    this->nav().faceToward(al::Vec3d(0, 0, 0));

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
    mAudioManager.scene()->setSpatializer<SPATIALIZER_TYPE>(speakers);
    mAudioManager.scene()->distanceAttenuation().law(al::ATTEN_NONE);
    mAudioManager.scene()->registerSynthClass<ChannelStrip>();
    registerDynamicScene(*mAudioManager.scene());
    mAudioManager.scene()->verbose(true);
    
    // Set fixed listener pose at origin (0,0,0)
    mAudioManager.setListenerPose(al::Pose(al::Vec3f(0, 0, 0)));

    // Add sound agents
    for (unsigned i = 0; i < 8; i++) {
      mAudioManager.addAgent(names[i].c_str());
      mAudioManager.agents()->at(i)->mInputChannel = i;
    }

    // prepare audio engine
    mAudioManager.prepare(audioIO());

    // prepare reference sphere
    al::addSphere(mSphereReference, 15);
    mSphereReference.primitive(al::Mesh::LINES);
  }

  void onCreate() override {
    // Set up GUI windows
    al::imguiInit();
    
    std::cout << "3D Sound Spatialization with GUI and Pickable Objects:" << std::endl;
    std::cout << "  1. Click on a sound source to show its control panel" << std::endl;
    std::cout << "  2. Click and drag objects to move them in space" << std::endl;
    std::cout << "  3. Camera can be moved with arrow keys (view only, doesn't affect audio)" << std::endl;
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

    mAudioManager.processAudio(io);
  }

  void onAnimate(double dt) override {
    mAudioManager.update();
  }

  void onDraw(al::Graphics& g) override {
    g.lens().eyeSep(0.0); // disable stereo rendering
    g.clear(0.0);
    al::gl::depthTesting(true); // necessary?

    mAudioManager.draw(g);
    mAudioManager.drawGUI(g);
    
    g.color(1.0);
    g.draw(mSphereReference);
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