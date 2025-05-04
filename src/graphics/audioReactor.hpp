#ifndef AUDIO_REACTOR_HPP
#define AUDIO_REACTOR_HPP


#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/FFT.h"
#include "Gamma/tbl.h"

#include <cstddef>
#include <vector>

/* TO DO:
- implement rms 
- implement onset detection 
- make sure everything is memory safe - already caught a seg fault issue
*/

/**
 * @brief Creates a spectrum analyzer. Has methods for retrieving centroid and flux. 
 * Need to call process in onSound. 
 * Call retrieval functions in onSound. Not useful to print / send values at audio rate.
 */

class SpectralListener {
public:
  gam::STFT stft;
  // mag spectrum at latest point in buffer
  std::vector<float> magnitudes;
  std::vector<float> prevMagnitudes;

  // initializer, creates members (arguments) before my constructor runs
  //commenting out constructor for now, seems to be causing memory leak / seg fault -
  SpectralListener() : stft(1024, 256, 0, gam::HAMMING) {
    stft.numAux(1); // 1 deals with mag spectrum
  }


  void process(float inputSample) {
    if (stft(inputSample)) { // if sample != null basically
      stft.spctToPolar();    // converts complex/ imaginary numbers to mag and
      // phase
      stft.copyBinsToAux(0, 0);  // copy magnitudes to auxilary buffer. there is
                                 // also an inverse function
      float *mags = stft.aux(0); // creates pointer to aux buffer
      if (mags){ //might not need this but seemed to be extra protection from seg fault on init - magnitudes are only assigned if there are values- no null pointer
      magnitudes.assign(mags, mags + stft.numBins());
      }
    }
  }

  const std::vector<float> &getMagnitudes() const {
    return magnitudes;
  } // reference to current magnitudes. might change this, but is currently
    // useful for comparing state in between frames. if process return a vector,
    // it would be constantly getting overwritten i think

  // std::vector<float> process(std::vector<float> &spectrum) {

  // decleration and implementation
  float getFlux() {
    if (prevMagnitudes.empty()) {
      prevMagnitudes = magnitudes;
      return 0.0f;
    }

    float flux = 0.0f;
    for (size_t i = 0; i < magnitudes.size(); ++i) {
      float delta = magnitudes[i] - prevMagnitudes[i];
      flux += std::max(delta, 0.0f); // only outputs positive values
    };
    prevMagnitudes = magnitudes;
    return flux;
  }
  // get spectral centroid
  float getCent() {
    if (magnitudes.empty())
      return 0.0f;

    float weightedSum = 0.0f;
    float magSum = 0.0f;

    float binFreq = stft.binFreq(); // Frequency spacing per bin
    for (size_t i = 0; i < magnitudes.size(); ++i) {
      float freq = i * binFreq;
      weightedSum += freq * magnitudes[i];
      magSum += magnitudes[i];
    }
      


    if (magSum == 0.0f) {
      return 0.0f;
      
    }
    return weightedSum / magSum;
  };

};

// class DynamicListener {
//   // gam::EnvFollow envfollow;
//   // gam::ZeroCross zrc;

// public:
//   void process();
// };

#endif