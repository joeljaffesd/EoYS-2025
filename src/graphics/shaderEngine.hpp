#ifndef EOYS_SHADER_ENGINE_HPP
#define EOYS_SHADER_ENGINE_HPP

// al includes
#include "al/ui/al_ParameterBundle.hpp"
#include "al/scene/al_PositionedVoice.hpp"
#include "al/ui/al_ControlGUI.hpp"

// giml includes
#include "../../../Gimmel/include/filter.hpp"

// eoys includes
#include "shaderToSphere.hpp"
#include "audioReactor.hpp" 
#include "vfxUtility.hpp"
#include "vfxMain.hpp"

#include "graphicsVoice.hpp"

class ShaderEngine : public GraphicsVoice {
private:
  ShadedSphere shaderSphere;
  SpectralListener specListen;
  DynamicListener dynListen;

  al::Parameter now {"now", "", 0.f, 0.f, std::numeric_limits<float>::max()};
  al::Parameter flux {"flux", "", 0.01f, 0.f, 1.f};
  al::Parameter centroid = {"centroid", "", 1.f, 0.f, 20000.f};
  al::Parameter rms = {"rms", "", 0.f, 0.f, 1.f};
  al::Parameter onsetIncrement = {"onsetIncrement", "", 0.f, 0.f, 100.f};
  al::ParameterInt mChannel = {"mChannel", "", 0, 0, 8};
  al::ParameterBundle mParams {"Uniforms"};
  al::ControlGUI mGUI;

  FloatReporter fluxReporter;
  FloatReporter centroidReporter;
  FloatReporter rmsReporter;
  giml::OnePole<float> mOnePole;
  giml::OnePole<float> mOnePoleCent;

public:

  al::ParameterBundle& params() {
    return mParams;
  }

  void init(bool isReplica = false) override {
    this->GraphicsVoice::init(isReplica); // call base class init

    if (ImGui::GetCurrentContext() == nullptr) {
      al::imguiInit();
    }

    dynListen.setSilenceThresh(0.1);
    mGUI << now << flux << centroid << rms << onsetIncrement << mChannel;
    mParams << now << flux << centroid << rms << onsetIncrement << mChannel;
    shaderSphere.setSphere(15.f, 1000);
    this->shader();
  }

  void shader(std::string shaderPath = "../src/shaders/Reactive-shaders/fractal1.frag") {
    shaderSphere.setShaders("../src/shaders/Reactive-shaders/standard.vert", shaderPath);
  }

  void update(double dt = 0) override {
    if (isReplica) { return; }// skip update for replicas
    now = now + float(dt);

    mOnePoleCent.setCutoff(15000, 60);
    centroid = mOnePoleCent.lpf(centroidReporter.reportValue());

    mOnePole.setCutoff(1000, 60);
    flux = mOnePole.lpf(fluxReporter.reportValue());

    if (dynListen.detectOnset()) {
      std::cout << "NEW ONSET" << std::endl;
      onsetIncrement = onsetIncrement + 0.1f;
    }
  }

  void onProcess(al::AudioIOData& io) {
    if (isReplica) { return; }// skip update for replicas
    for (auto sample = 0; sample < io.framesPerBuffer(); sample++) {
      const float in = io.in(mChannel, sample);
      specListen.process(in);
      dynListen.process(in);
      centroidReporter.write(in);
      fluxReporter.write(in);
      rmsReporter.write(in);
    }
  }

  void onProcess(al::Graphics& g) override {
    // activate shader mode
    g.shader(shaderSphere.shader());

    // set unforms
    shaderSphere.setUniformFloat("u_time", now);
    shaderSphere.setUniformFloat("onset", onsetIncrement);
    shaderSphere.setUniformFloat("cent", centroid);
    shaderSphere.setUniformFloat("flux", flux);

    // draw
    shaderSphere.draw(g);

    // draw GUI 
    mGUI.draw(g); 
  }

};

#endif // EOYS_SHADER_ENGINE_HPP