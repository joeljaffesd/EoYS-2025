#ifndef EOYS_GRAPHICS_VOICE
#define EOYS_GRAPHICS_VOICE

#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_AudioIO.hpp"

class GraphicsVoice {
protected:
  bool isReplica = false;

public:
  GraphicsVoice() {}
  ~GraphicsVoice() {}
  inline virtual void init(bool isReplica = false) { this->isReplica = isReplica; }
  inline virtual void update(double dt = 0) {}
  inline virtual void onProcess(al::Graphics& g) {}
  inline virtual void onProcess(al::AudioIOData& io) {}
};

#endif