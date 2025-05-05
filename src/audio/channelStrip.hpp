#ifndef EOYS_CHANNEL_STRIP
#define EOYS_CHANNEL_STRIP

#include "../../Gimmel/include/gimmel.hpp"

#include "effectsEngine.hpp"
#include "spatialAgent.hpp"

/**
 * @brief TODO
 * @todo Finish and encapsulate GUI
 */
class ChannelStrip : public EffectsEngine, public SpatialAgent {
private:
  al::ParameterBool enabled { "Enabled", "", false };
  al::ParameterInt mInputChannel { "Input Channel", "", 0, 0, 8 };
  al::Parameter mGain { "Gain", "", 0.f, -96.f, 12.f };
  al::Parameter mVolume { "Volume", "", -96.f, -96.f, 12.f };
  al::ParameterBundle mBasics { "Basics" };
  
public:
  void init() {
    mBasics << enabled << mInputChannel << mGain << mVolume;
    mGui << mBasics;
    SpatialAgent::init();
    
    std::unique_ptr<giml::Chorus<float>> chorus = std::make_unique<giml::Chorus<float>>(48000);
    this->addEffect(std::move(chorus));

  }

  // TODO... reconcile inheritance pattern
  void onProcess(al::AudioIOData& io) final {
    if (!enabled) { return; }
    for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
      float input = io.in(mInputChannel, sample) * giml::dBtoA(mGain); 
      //float output = this->processSample(input) * giml::dBtoA(mVolume);
      float output = al::rnd::uniformS() * giml::dBtoA(mVolume);
      io.out(0, sample) = output;
    }
  }

};
#endif // EOYS_CHANNEL_STRIP
