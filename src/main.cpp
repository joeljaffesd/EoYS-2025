// Single macro to switch between desktop and Allosphere configurations
// #define DESKTOP

#ifdef DESKTOP
  // Desktop configuration
  #define SAMPLE_RATE 48000
  #define AUDIO_CONFIG SAMPLE_RATE, 128, 2, 8
  #define SPATIALIZER_TYPE al::AmbisonicsSpatializer
  #define SPEAKER_LAYOUT al::StereoSpeakerLayout()
#else
  // Allosphere configuration
  #define SAMPLE_RATE 44100
  #define AUDIO_CONFIG SAMPLE_RATE, 256, 60, 9
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

#include "../assets/namModels/BassModel.h"
#include "../assets/namModels/MarshallModel.h"

// Gamma ig for now
#include "Gamma/SamplePlayer.h"

class Main : public al::DistributedApp {
public:
  int prevVoiceId, newVoiceId;
  AudioManager<ChannelStrip> mManager;
  al::ParameterBool mAudioMode {"mAudioMode", "", false};
  al::ParameterBool mMute {"mMute", "", true}; // mute by default

  gam::SamplePlayer<float, gam::ipl::Cubic, gam::phsInc::Loop> player[8];
  std::vector<std::string> names;

  al::ParameterInt mSceneIndex{"mSceneIndex", "", -1};
  std::vector<std::function<void()>> mCallbacks;

  template <class TSynthVoice>
  TSynthVoice* loadVoice(bool offset = true) {
    mManager.scene()->triggerOff(prevVoiceId);
    auto* oldVoice = dynamic_cast<al::PositionedVoice*>(mManager.scene()->getActiveVoices());
    // oldVoice->setPose(al::Pose(al::Vec3d(0, 0, 0)));
    oldVoice->setPose( al::Pose( oldVoice->pose().vec() - al::Vec3d(0, 30, 0) ) );

    // if shader, reset it
    if (auto* shaderVoice = dynamic_cast<ShaderEngine*>(oldVoice)) {
      shaderVoice->reset();
    }

    // if video, restart it
    if (auto* videoVoice = dynamic_cast<VideoSphereLoaderCV*>(oldVoice)) {
      videoVoice->restart();
    }

    auto* voice = mManager.scene()->getVoice<TSynthVoice>();
    if (offset) {
      voice->setPose(al::Pose(al::Vec3d(0, 30, 0)));
    }
    prevVoiceId = newVoiceId;
    newVoiceId = mManager.scene()->triggerOn(voice);
    return voice;
  }

  void onInit() override {

    // misc
    mCallbacks.push_back([this]() {
      loadVoice<ImageSphereLoader>();
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<AssetEngine>();
    });      

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/misc/eye.mp4");
    }); 

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/misc/charcoal.mp4");
      voice->setSpeed(0.5f);
      voice->toggleRotation(true);
    });      
    
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ShaderEngine>();
      voice->shaderPath("../src/shaders/fractal3.frag");
    });        

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ShaderEngine>();
      voice->shaderPath("../src/shaders/metaBall1.frag");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ShaderEngine>();
      voice->shaderPath("../src/shaders/Psych1.frag");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ShaderEngine>();
      voice->shaderPath("../src/shaders/Psych2.frag");
    });  


    // xanadu
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ImageSphereLoader>();
      voice->setImageFilePath("../assets/scenes/xanadu/01.png");
      voice->toggleRotation(true);
    }); 

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/xanadu/02.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ImageSphereLoader>();
      voice->setImageFilePath("../assets/scenes/xanadu/03.png");
      voice->toggleRotation(true);
    });    

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/xanadu/04.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/xanadu/05.mp4");
    });   


    // runnin w/ the devil 
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ShaderEngine>();
      voice->shaderPath("../src/shaders/flame.frag");
    });


    // eruption 
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ShaderEngine>();
      voice->setInputChannel(1);
      voice->shaderPath("../src/shaders/julia.frag");
    });    


    // TODO you really got me


    // manic depression 
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manicDepression/01.mp4");
      voice->toggleRotation(true);
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manicDepression/02.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manicDepression/03.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manicDepression/04.mp4");
    });    


    // TODO texas flood 

    
    // red barchetta 
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/redBarchetta/01.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/redBarchetta/02.mp4");
    });         

    // buggy drive sequence 
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<AssetEngine>();
      voice->toggleRotation(false);
      voice->setAssetFilePath("../assets/3dModels/car");
    });  
    
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<AssetEngine>();
      voice->toggleRotation(false);
      voice->setPose(al::Pose(voice->pose().vec() + al::Vec3d(0, -0.3, -0.2)));
      voice->setAssetFilePath("../assets/3dModels/car");
    });  
    
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<ShaderEngine>(false);
      voice->setInputChannel(1);
      voice->shaderPath("../src/shaders/julia.frag");
    });  

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<AssetEngine>();
      voice->toggleRotation(false);
      voice->setAssetFilePath("../assets/3dModels/car");
    });     


    // TODO whipping post 


    // man in the box
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manInTheBox/01.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manInTheBox/02.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manInTheBox/03.mp4");
      voice->toggleRotation(true);
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manInTheBox/04.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manInTheBox/05.mp4");
      voice->setSpeed(0.5);
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/manInTheBox/06.mp4");
    });  


    // TODO dazed and confused       

    
    // the pot
    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/thePot/01.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/thePot/02.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/thePot/03.mp4");
    });

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/thePot/04.mp4");
    });       

    mCallbacks.push_back([this]() {
      auto* voice = loadVoice<VideoSphereLoaderCV>();
      voice->setVideoFilePath("../assets/scenes/thePot/05.mp4");
    });        

    // TODO thank you slide


    al::imguiInit();

    mManager.scene()->verbose(true);
    this->registerDynamicScene(*mManager.scene());

    mManager.scene()->registerSynthClass<ImageSphereLoader>();
    mManager.scene()->registerSynthClass<AssetEngine>();
    mManager.scene()->registerSynthClass<ShaderEngine>();
    mManager.scene()->registerSynthClass<VideoSphereLoaderCV>();

    player[0].load("../assets/wavFiles/vocals.wav");
    player[1].load("../assets/wavFiles/guitar.wav");
    player[2].load("../assets/wavFiles/bass.wav");
    player[3].load("../assets/wavFiles/kick.wav");
    player[4].load("../assets/wavFiles/snare.wav");
    player[5].load("../assets/wavFiles/floorTom.wav");
    player[6].load("../assets/wavFiles/midTom.wav");
    player[7].load("../assets/wavFiles/highTom.wav");

    names.push_back("Vocals");
    names.push_back("Guitar1");
    names.push_back("Guitar2");
    names.push_back("Guitar3");
    names.push_back("Bass");
    names.push_back("Kick");
    names.push_back("Snare");
    names.push_back("Floor Tom");
    names.push_back("Mid Tom");
    names.push_back("High Tom");

    // Add sound agents
    for (unsigned i = 0; i < 10; i++) {
      if (isPrimary()) { mManager.addAgent(names[i].c_str()); }
      else { mManager.addAgent(names[i].c_str(), false); } // mark as replica if not primary
      if (i < 2) { 
        mManager.agents()->at(i)->mInputChannel = i; 
      } else if (i == 2) {
        mManager.agents()->at(i)->mInputChannel = i - 1;
      } else {
        mManager.agents()->at(i)->mInputChannel = i - 2;
      }
    }

    // todo make this not suck
    mManager.agents()->at(0)->set(0.0, 90.0, 7.5, 1.0);
    mManager.agents()->at(1)->set(-60, 0.0, 5.0, 1.0);
    mManager.agents()->at(2)->set(60, 0.0, 5.0, 1.0); 
    mManager.agents()->at(3)->set(180, 0.0, 5.0, 1.0);  
    mManager.agents()->at(4)->set(0.0, -90.0, 5.0, 1.0);
    mManager.agents()->at(5)->set(0.0, -90.0, 1.0, 1.0);
    mManager.agents()->at(6)->set(0.0, 90.0, 3.5, 1.0);
    mManager.agents()->at(7)->set(0.0, 30.0, 8.0, 1.0);    // 0 degrees
    mManager.agents()->at(8)->set(120.0, 30.0, 8.0, 1.0);  // 120 degrees
    mManager.agents()->at(9)->set(240.0, 30.0, 8.0, 1.0);  // 240 degrees

    // vocal fx
    mManager.agents()->at(0)->addEffect<giml::Compressor<float>, SAMPLE_RATE>();
    mManager.agents()->at(0)->addEffect<giml::Delay<float>, SAMPLE_RATE>();
    mManager.agents()->at(0)->addEffect<giml::Reverb<float>, SAMPLE_RATE>();
    mManager.agents()->at(0)->updateParameters();

    // gtr fx
    for (auto i = 1; i < 4; i++) {
      mManager.agents()->at(i)->addAmp<float, MarshallModelLayer1, MarshallModelLayer2, MarshallModelWeights>();
      mManager.agents()->at(i)->addEffect<giml::Detune<float>, SAMPLE_RATE>();
      mManager.agents()->at(i)->addEffect<giml::Delay<float>, SAMPLE_RATE>();
      mManager.agents()->at(i)->addEffect<giml::Reverb<float>, SAMPLE_RATE>();
      mManager.agents()->at(i)->updateParameters();
    }

    // bass fx
    mManager.agents()->at(4)->addAmp<float, BassModelLayer1, BassModelLayer2, BassModelWeights>();
    mManager.agents()->at(4)->addEffect<giml::Compressor<float>, SAMPLE_RATE>();
    mManager.agents()->at(4)->updateParameters();

    // drums fx
    for (auto i = 5; i < 10; i++) {
      mManager.agents()->at(i)->addEffect<giml::Compressor<float>, SAMPLE_RATE>();
      mManager.agents()->at(i)->addEffect<giml::Reverb<float>, SAMPLE_RATE>();
      mManager.agents()->at(i)->updateParameters();
    }

    // preset handlers
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
      // // for now... write audio files to input to simulate audio input
      // for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
      //   float input[8];
      //   for (auto index = 0; index < 8; index++) {
      //     input[index] = player[index]();
      //     io.inW(index, sample) = input[index];
      //   }
      // }

      mManager.processAudio(io);

      if (mMute) {
        for (auto channel = 0; channel < io.channelsOut(); channel++) {
          for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
            io.out(channel, sample) = 0.f;
          }
        }
      }

      // monitor mix
      for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
        unsigned mon1 = 15, mon2 = 14, mon3 = 13, mon4 = 12;
        unsigned sub = 47;

        int now = int(io.framesPerBuffer()) - sample;

        for (auto mon = 12; mon < 16; mon++) {
          io.out(mon, sample) = io.in(8, sample);
        }

        if (!mMute) {
          // feed sub, bass and kick
          io.out(sub, sample) = mManager.agents()->at(4)->buffer().readSample(now); // bass
          io.out(sub, sample) += mManager.agents()->at(5)->buffer().readSample(now); // kick
        }

        // // vox
        io.out(mon1, sample) += mManager.agents()->at(0)->buffer().readSample(now);

        // // gtr mixdown
        io.out(mon2, sample) += mManager.agents()->at(1)->buffer().readSample(now) +
                                mManager.agents()->at(2)->buffer().readSample(now) +
                                mManager.agents()->at(3)->buffer().readSample(now); 

        // bass
        io.out(mon3, sample) += mManager.agents()->at(4)->buffer().readSample(now);

        // // drums mixdown
        io.out(mon4, sample) += mManager.agents()->at(5)->buffer().readSample(now) +
                                mManager.agents()->at(6)->buffer().readSample(now) +
                                mManager.agents()->at(7)->buffer().readSample(now) +
                                mManager.agents()->at(8)->buffer().readSample(now) +
                                mManager.agents()->at(9)->buffer().readSample(now);
      }

    }
  }

  void onAnimate(double dt) override {
    if (mAudioMode) {
      mManager.update(dt);
    } else {
      mManager.scene()->update(dt);
    }
  }

  int phase = 0;
  bool onKeyDown(const al::Keyboard& k) override {
    if (isPrimary()) {
      if (k.key() == ']') { 
        mSceneIndex = mSceneIndex + 1; 
        std::cout << "Scene index: " << mSceneIndex << std::endl;
        if (mSceneIndex >= 0 && mSceneIndex < mCallbacks.size()) {
          mCallbacks[mSceneIndex]();
        }
      }
      else if (k.key() == '[') { 
        mSceneIndex = mSceneIndex - 1; 
        std::cout << "Scene index: " << mSceneIndex << std::endl;
        if (mSceneIndex >= 0 && mSceneIndex < mCallbacks.size()) {
          mCallbacks[mSceneIndex]();
        }
      }
      else if (k.key() == 'm') { mMute = !mMute; }
      else if (k.key() == 'g') { mAudioMode = !mAudioMode; }
      else if (k.key() == 's') { mManager.storePresets(); }
    }
    return true;
  }

  void onDraw(al::Graphics& g) override {
    g.lens().eyeSep(0.0); // disable stereo rendering
    g.clear(0);
    mManager.draw(g);
    if (isPrimary()) {
      mManager.draw(g);
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
