#ifndef EOYS_CHANNEL_STRIP
#define EOYS_CHANNEL_STRIP

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 48000
#endif

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
  giml::CircularBuffer<float> mBuffer; // store some signal history 
  
public:

  void init() {
    mBuffer.allocate(1024);
    mBasics << enabled << mInputChannel << mGain << mVolume;
    mGui << mBasics;
    this->registerParameters(enabled, mInputChannel, mGain, mVolume);
    
    mGui << this->mParamBundles[0]; // can add effects after this is called... sometimes.

    this->updateParameters(); // register all parameters in the bundle

    SpatialAgent::init();
    registerParameters(this->mPose); // need for distributed.
  }

  // call after adding fx/amps to an agent
  void updateParameters() {
    for (auto& param : mParamBundles[0].parameters()) {
      auto paramPtr = static_cast<al::Parameter*>(param);
      this->registerParameter(*paramPtr);
    }
  }

  // TODO... reconcile inheritance pattern
  void onProcess(al::AudioIOData& io) final {
    if (!enabled) { return; }
    for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
      float input = io.in(mInputChannel, sample) * giml::dBtoA(mGain); 
      float output = this->processSample(input) * giml::dBtoA(mVolume);
      mBuffer.writeSample(output);
      io.out(0, sample) = mBuffer.readSample(0); // read last in
    }
  }

  // useful for getting signal history
  giml::CircularBuffer<float>& buffer() {
    return mBuffer;
  }

};
#endif // EOYS_CHANNEL_STRIP
