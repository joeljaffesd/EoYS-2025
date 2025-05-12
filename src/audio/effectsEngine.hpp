#ifndef EOYS_EFFECTS_ENGINE
#define EOYS_EFFECTS_ENGINE

// std includes
#include <vector>
#include <unordered_set>

// al includes
#include "al/io/al_AudioIOData.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/app/al_GUIDomain.hpp"

// giml includes
#include "../Gimmel/include/gimmel.hpp"

/**
 * @brief Basic encapsulation of the an fx chain / inserts, using `Gimmel`
 */
class EffectsEngine {
private:
  std::vector<std::unique_ptr<giml::Effect<float>>> mEffects;
  giml::EffectsLine<float> mEffectsLine;

public:
  void addEffect(std::unique_ptr<giml::Effect<float>> effect) {
    mEffects.push_back(std::move(effect));
    mEffectsLine.pushBack(mEffects.back().get());
  }

  giml::EffectsLine<float>& getEffectsLine() {
    return mEffectsLine;
  }

  // TODO: Effect Params & GUI

  float processSample(float& input) {
    return mEffectsLine.processSample(input);
  }
};

#endif // EOYS_EFFECTS_ENGINE
