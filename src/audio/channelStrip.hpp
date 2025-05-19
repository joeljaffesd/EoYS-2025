#ifndef EOYS_CHANNEL_STRIP
#define EOYS_CHANNEL_STRIP

#include "../../Gimmel/include/gimmel.hpp"
#include "ampModeler.hpp"
// #include "../../assets/namModels/MarshallModel.h"
#include "../../assets/namModels/BassModel.h"

#include "effectsEngine.hpp"
#include "spatialAgent.hpp"

/**
 * @brief TODO
 * @todo Finish and GUI and params
 */
class ChannelStrip : public EffectsEngine, public SpatialAgent {
public:
  al::ParameterBool enabled { "Enabled", "", true };
  al::ParameterInt mInputChannel { "Input Channel", "", 0, 0, 7 };
  al::Parameter mGain { "Gain", "", 0.f, -96.f, 12.f };
  al::Parameter mVolume { "Volume", "", 0.f, -96.f, 12.f };
  al::ParameterBundle mBasics { "Basics" }; 
  
public:

  void init() {
    mBasics << enabled << mInputChannel << mGain << mVolume;
    mGui << mBasics;

    // this is wrought with bugs...
    // seems that a break occurs, but stuff added after the break works
    // this break may be.. 3 fx that all have a "depth" param?
    this->addEffect<giml::Detune<float>, 48000>();                 
    this->addEffect<giml::Chorus<float>, 48000>(); // depth breaks at 20+ms rn
    this->addEffect<giml::Flanger<float>, 48000>();
    this->addEffect<giml::Expander<float>, 48000>();
    this->addEffect<giml::Tremolo<float>, 48000>();
    this->addAmp<float, BassModelLayer1, BassModelLayer2, BassModelWeights>();
    this->addEffect<giml::Phaser<float>, 48000>();
    this->addEffect<giml::Compressor<float>, 48000>();
    this->addEffect<giml::Delay<float>, 48000>();
    this->addEffect<giml::Reverb<float>, 48000>();
    //auto* reverb = dynamic_cast<giml::Reverb<float>*>(this->mEffects.back().get());
    //reverb->setParams(0.1f, 0.3f, 0.9f, 0.5f, 50.f, 0.9f, giml::Reverb<float>::RoomType::SPHERE);
    
    mGui << this->mParamBundles[0]; // can add effects after this is called... sometimes.

    SpatialAgent::init();
    registerParameters(this->mPose); // need for distributed.
  }

  // TODO... reconcile inheritance pattern
  void onProcess(al::AudioIOData& io) final {
    if (!enabled) { return; }
    for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
      float input = io.in(mInputChannel, sample) * giml::dBtoA(mGain); 
      float output = this->processSample(input) * giml::dBtoA(mVolume);
      io.out(0, sample) = output;
    }
  }

};
#endif // EOYS_CHANNEL_STRIP
