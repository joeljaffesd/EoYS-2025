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
#include "ampModeler.hpp"

/**
 * @brief Basic encapsulation of the an fx chain / inserts, using `Gimmel`
 */
class EffectsEngine {
public:
  std::vector<std::unique_ptr<giml::Effect<float>>> mEffects;
  giml::EffectsLine<float> mEffectsLine;

  // param handling 
  std::vector<std::shared_ptr<al::ParameterMeta>> mParams;
  std::vector<al::ParameterBundle> mParamBundles; 

public:
  EffectsEngine() {
    mParamBundles.push_back(al::ParameterBundle("Effects"));
  }

  template <typename T, typename Layer1, typename Layer2, class TWeights>
  void addAmp() {
    mEffects.push_back(std::make_unique<giml::AmpModeler<T, Layer1, Layer2>>());
    mEffectsLine.pushBack(mEffects.back().get());
    auto* amp = dynamic_cast<giml::AmpModeler<T, Layer1, Layer2>*>(mEffects.back().get());
    TWeights mWeights;
    amp->loadModel(mWeights.weights);

    mParams.push_back(std::make_shared<al::ParameterBool>("Amp Enabled", "", false));
    // Get a pointer to the ParameterBool
    auto* theToggle = dynamic_cast<al::ParameterBool*>(mParams.back().get());
    if (theToggle) {
      auto* effectPtr = mEffects.back().get(); // Get pointer to current effect
      theToggle->registerChangeCallback([effectPtr](float value) {
        bool enabled = value > 0.5f;
        effectPtr->toggle(enabled);
      });
    }
    mParamBundles.back().addParameter(mParams.back().get());

  }

  template<class TEffect, int SampleRate>
  void addEffect() {
    mEffects.push_back(std::make_unique<TEffect>(SampleRate));
    mEffectsLine.pushBack(mEffects.back().get());
    auto effectName = al::demangle(typeid(TEffect).name());

    // TODO programmatic attach of effect params to GUI
    size_t indexStart = mParams.size();
    mParams.push_back(std::make_shared<al::ParameterBool>(effectName + "Enabled", "", false));
    // Get a pointer to the ParameterBool
    auto* theToggle = dynamic_cast<al::ParameterBool*>(mParams.back().get());
    if (theToggle) {
      auto* effectPtr = mEffects.back().get(); // Get pointer to current effect
      theToggle->registerChangeCallback([effectPtr](float value) {
        bool enabled = value > 0.5f;
        effectPtr->toggle(enabled);
      });
    }

    for (auto* param : mEffects.back()->getParams()) {
      switch (param->type) {
        case giml::Param<float>::TYPE::CONTINUOUS:
          mParams.push_back(std::make_shared<al::Parameter>(
            effectName + param->name, "", param->def, param->min, param->max
          ));
          break;
        case giml::Param<float>::TYPE::CHOICE:
          mParams.push_back(std::make_shared<al::ParameterInt>(
            effectName + param->name, "", (int)param->def, (int)param->min, (int)param->max
          ));
          break;
        case giml::Param<float>::TYPE::BOOL:
          mParams.push_back(std::make_shared<al::ParameterBool>(
            effectName + param->name, "", (bool)param->def
          ));
          break;
      }

      // param callback
      auto* theParam = dynamic_cast<al::Parameter*>(mParams.back().get());
      if (theParam) {
        std::string fullParamName = theParam->getName(); // e.g. "giml::Detune<float>pitchRatio"
        
        // Extract just the parameter name without the effect prefix
        std::string effectPrefix = effectName; // e.g. "giml::Detune<float>"
        std::string paramName = fullParamName.substr(effectPrefix.length()); // "pitchRatio"
        
        auto* effectPtr = mEffects.back().get();
        theParam->registerChangeCallback([effectPtr, paramName](float value) {
          effectPtr->setParam(paramName, value);
          effectPtr->updateParams();
        });
      }
      
    }

    // mParamBundles.push_back(al::ParameterBundle());
    for (size_t i = indexStart; i < mParams.size(); i++) {
      mParamBundles.back().addParameter(mParams[i].get());
    }
    //mParamBundles[0].addBundle(mParamBundles.back(), " " + al::demangle(typeid(TEffect).name()));
  }

  giml::EffectsLine<float>& getEffectsLine() {
    return mEffectsLine;
  }

  float processSample(float& input) {
    return mEffectsLine.processSample(input);
  }
};

#endif // EOYS_EFFECTS_ENGINE
