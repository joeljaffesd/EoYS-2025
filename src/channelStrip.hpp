#ifndef EOYS_CHANNEL_STRIP_HPP
#define EOYS_CHANNEL_STRIP_HPP

#include <vector>
#include "al/io/al_AudioIOData.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "../Gimmel/include/gimmel.hpp"

/**
 * @brief TODO
 */
class SpatialEngine {
private:
  al::Pose listenerPose;
  al::Pose sourcePose;
  al::Spatializer* spatializer;
  al::Speakers speakerLayout;

public:
  SpatialEngine() : spatializer(nullptr) {}

  void init() {} // TODO

  /**
   * @brief TODO
   */
  void processAudio(al::AudioIOData& io) {
    if (spatializer) {
      spatializer->prepare(io);
      spatializer->renderBuffer(io, sourcePose.pos(), io.busBuffer(0),
                                io.framesPerBuffer());
      spatializer->finalize(io);
    }
  }

}; // TODO

/**
 * @brief TODO
 */
class EffectsEngine {
private:
  std::vector<std::unique_ptr<giml::Effect<float>>> effects;

public:
  void addEffect(std::unique_ptr<giml::Effect<float>> effect) {
    effects.push_back(std::move(effect));
  }

  void processAudio(float& sample) {
    for (auto& effect : effects) {
      sample = effect->processSample(sample);
    }
  }
};

/**
 * @brief TODO
 */
class ChannelStrip : SpatialEngine, EffectsEngine {
private:
  
public:
  void processAudio(al::AudioIOData& io) {
    // TODO
  }
};
#endif
