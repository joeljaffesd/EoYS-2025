#ifndef AUDIO_REACTOR_HPP
#define AUDIO_REACTOR_HPP


#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/FFT.h"
#include "Gamma/tbl.h"
//#include "Gamma/"

#include <cmath>
#include <cstddef>
#include <vector>

/* TO DO:
- implement rms 
- implement onset detection -- use silence detected from gamma
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

  SpectralListener() : stft(1024, 256, 0, gam::HAMMING) {
    stft.numAux(1); // 1 deals with mag spectrum
  }

/**
 * @brief Call in on sound. Pass in input samples
 */
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

  // decleration and implementation
/** 
* @brief Store in a var or pass into param. Measures difference in freq spectrum magnitude between frames.
*/
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
  /** 
* @brief Store in a var or pass into param. Measures center of mass of current freq.
*/
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

/** 
* @brief Creates a dynamics analyzer. Has methods for rms, more to come. 
* Need to call process in onSound. 
* Call retrieval functions in onSound. Not useful to print / send values at audio rate.
*/

class DynamicListener {
  public:
  //not using yet
  // gam::EnvFollow<float> env{0.1};
  // gam::ZeroCross<float> zrc;

  float currentRMS;
  float sumOfSquares;
  float sampleCounter;
/** 
* @brief call in onSound. pass in input sample
*/
  void process(float inputSample){
    // env(inputSample);
    // zrc(inputSample); 

    sumOfSquares += inputSample * inputSample; //squaring raw input sample value and summing.
    sampleCounter++;

  }
  /** 
* @brief call in onSound. returns float of up to date rms
*/
  float getRMS(){
    if (sampleCounter > 0){
    currentRMS = std::sqrt(sumOfSquares / sampleCounter);
    }
    // else{
    //   currentRMS = 0.0f;
    // }
    return currentRMS;
  
  }
  void resetRMS(){
    currentRMS = 0.0f;
    sampleCounter = 0;
    sumOfSquares = 0.0f;
  }
 
};

#endif
