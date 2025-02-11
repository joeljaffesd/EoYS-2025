#define NAM_SAMPLE_FLOAT
#include "al/app/al_DistributedApp.hpp"
#include "../NeuralAmpModelerCore/NAM/all.h"

struct State {
  int ignore = 0;
};

class NamTest : public al::DistributedAppWithState<State> {
private:
  std::unique_ptr<nam::DSP> mModel; 

public:
  void onInit() override {
    if (isPrimary()) { // load NAM model on primary
      const char* mPath = "../../resources/MarshallModel.nam";
      mModel = nam::get_dsp(mPath);
      mModel->ResetAndPrewarm(this->audioIO().framesPerSecond(), 1);
    }
  }

  void onSound(al::AudioIOData& io) override {
    if (isPrimary()) { // only do audio for primary
      for (int sample = 0; sample < io.framesPerBuffer(); sample++) {
        // calculate output from input
        float input = io.in(0, sample);
        float output = 0.f;
        mModel->process(&input, &output, 1);

        // write output to all output channels
        for (int channel = 0; channel < io.channelsOut(); channel++) {
          io.out(channel, sample) = output;
        }
      }
    }  
  }
};

int main() {
  NamTest mNamTest;
  if (mNamTest.isPrimary()) {
    mNamTest.configureAudio(48000, 128, 2, 1);
  }
  mNamTest.start();
  return 0;
}