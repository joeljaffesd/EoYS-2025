#ifndef EOYS_CHANNEL_STRIP_HPP
#define EOYS_CHANNEL_STRIP_HPP

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

  // TODO: Effect Params & GUI

  // TODO fix mixing strategy
  // void processAudio(al::AudioIOData& io) {
  //   while(io()) {
  //     float sample = io.in(0);
  //     sample = mEffectsLine.processSample(sample);
  //     io.out(0) = sample;
  //   }
  // }

  float processSample(float input) {
    input = mEffectsLine.processSample(input);
  }
};

/**
 * @brief TODO
 * @todo Finish and encapsulate GUI
 */
class ChannelStrip : public SpatialEngine, public EffectsEngine {
private:
  al::ParameterBool enabled { "Enabled", "", true };
  al::ParameterInt mInputChannel { "Input Channel", "", 0, 0, 1 };
  al::Parameter mGain { "Gain", "", 0.f, -96.f, 12.f };
  al::Parameter mVolume { "Volume", "", 0.f, -96.f, 12.f };
  al::ParameterBundle mBasics { "Basics" };

  al::Parameter mDelayTime { "Delay Time", "", 0.f, 0.f, 1.f };
  al::Parameter mDelayFeedback { "Delay Feedback", "", 0.f, 0.f, 1.f };
  al::ParameterBundle mDelay { "Delay" };

  al::Parameter mReverbTime { "Reverb Time", "", 0.f, 0.f, 1.f };
  al::Parameter mReverbFeedback { "Reverb Feedback", "", 0.f, 0.f, 1.f };
  al::ParameterBundle mReverb { "Reverb" };

  al::ParameterMenu mTabs{"Tabs", "", 0};
  std::vector<al::ParameterBundle*> mBundles;

  int drawTabBar(al::ParameterMenu &menu) {
    auto tabNames = menu.getElements();

    // Draw tab headers
    ImGui::Columns(tabNames.size(), nullptr, true);
    for (size_t i = 0; i < tabNames.size(); i++) {
      if (ImGui::Selectable(tabNames[i].c_str(), menu.get() == i)) {
        menu.set(i);
      }
      ImGui::NextColumn();
    }
    // Draw tab contents
    ImGui::Columns(1);
    return menu.get();
  }
  
public:
  void init() {
    mTabs.setElements({"Basics", "Inserts", "Spatialization"});
    // Add parameters to bundles (tabs)
    mBasics << enabled << mInputChannel << mGain << mVolume;
    mDelay << mDelayTime << mDelayFeedback;
    mReverb << mReverbTime << mReverbFeedback;

    // Add bundles to the GUI
    this->addBundle(mBasics);
    this->addBundle(mDelay);
    this->addBundle(mReverb);

    // Initialize ImGui
    al::imguiInit();
  }

  void update() {
    al::imguiBeginFrame();
    ImGui::SetNextWindowBgAlpha(0.9);

    ImGui::Begin("Channel Name");

    if (ImGui::Button("Add Tab")) {
      auto elements = mTabs.getElements();
      elements.push_back("new " + std::to_string(mTabs.getElements().size()));
      mTabs.setElements(elements);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Tab")) {
      auto elements = mTabs.getElements();
      if (!elements.empty()) {
        elements.pop_back();
        mTabs.setElements(elements);
      }
    }

    // Draw tab bar
    int currentTab = drawTabBar(mTabs);

    // Draw tab contents according to currently selected tab
    switch (currentTab) {
    case 0:
      al::ParameterGUI::drawBundle(mBundles[0]);
      break;
    case 1:
      for (int bundle = 1; bundle < mBundles.size(); bundle++) {
        al::ParameterGUI::drawBundle(mBundles[bundle]);
      }
      break;
    case 2:
      ImGui::Text("Spatialization");
      break;
    default:
      ImGui::Text(
          "%s",
          std::string("Dynamic Tab: " + std::to_string(mTabs.get())).c_str());
      break;
    }

    ImGui::End();
    al::imguiEndFrame();
  }

  void draw(al::Graphics &g) {
    al::imguiDraw();
  }

  void addBundle(al::ParameterBundle &bundle) {
    mBundles.push_back(&bundle);
  }

  // TODO (placeholder multi-mono for now)
  void processAudio(al::AudioIOData& io) {
    while(io()) {
      float input = io.in(0);
      float output = this->processSample(input);
      for (int channel = 0; channel < io.channelsOut(); channel++) {
        io.out(channel) = output;
      }
    }
  }

};
#endif
