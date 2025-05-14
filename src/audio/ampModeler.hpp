#ifndef EOYS_AMP_MODELER
#define EOYS_AMP_MODELER

#include "../../Gimmel/include/utility.hpp"

// Add NAM compatibility to giml
namespace giml {
  template<typename T, typename Layer1, typename Layer2>
  class AmpModeler : public Effect<T>, public wavenet::RTWavenet<1, 1, Layer1, Layer2> {
  public:
    T processSample(const T& input) override {
      if (!this->enabled) { return input; }
      return this->model.forward(input);
    }
  };
} // namespace giml


#endif EOYS_AMP_MODELER