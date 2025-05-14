#define SPATIALIZER_TYPE al::AmbisonicsSpatializer // for Ambisonics
#ifndef SPATIALIZER_TYPE
#define SPATIALIZER_TYPE al::Lbap // for playback in Allosphere
#endif

#define AUDIO_CONFIG 48000, 128, 2, 0 // for lap/desktop computer 
#ifndef AUDIO_CONFIG
#define AUDIO_CONFIG 44100, 256, 60, 0 // for playback in Allosphere
#endif

#define SPEAKER_LAYOUT al::StereoSpeakerLayout() // for lap/desktop computer 
#ifndef SPEAKER_LAYOUT
#define SPEAKER_LAYOUT al::AlloSphereSpeakerLayoutCompensated() // for playback in Allosphere
#endif

#include "al/app/al_DistributedApp.hpp"
#include "al/scene/al_DistributedScene.hpp"
#include "../../audio/audioManager.hpp"

struct MyApp : public al::DistributedApp {
  AudioManager<ChannelStrip> mAudioManager;

  void onInit() override {

    // TODO: encapsulate this in a function
    auto speakers = SPEAKER_LAYOUT; 
    mAudioManager.scene()->setSpatializer<SPATIALIZER_TYPE>(speakers);
    mAudioManager.scene()->distanceAttenuation().law(al::ATTEN_INVERSE_SQUARE);
    registerDynamicScene(*mAudioManager.scene());
    mAudioManager.scene()->verbose(true);
    
    // Set fixed listener pose at origin (0,0,0)
    mAudioManager.setListenerPose(al::Pose(al::Vec3f(0, 0, 0)));

    // Add sound agents
    for (unsigned i = 0; i < 3; i++) {
      mAudioManager.addAgent("Spatial Agent");
    }

    // prepare audio engine
    mAudioManager.prepare(audioIO());

  }

  void onCreate() override {
    // Set up GUI windows
    al::imguiInit();
    
    // Set up camera - still movable but doesn't affect audio
    nav().pos(0, 0, 10);
    
    std::cout << "3D Sound Spatialization with GUI and Pickable Objects:" << std::endl;
    std::cout << "  1. Click on a sound source to show its control panel" << std::endl;
    std::cout << "  2. Click and drag objects to move them in space" << std::endl;
    std::cout << "  3. Camera can be moved with arrow keys (view only, doesn't affect audio)" << std::endl;
  }
  
  void onAnimate(double dt) override {
    mAudioManager.update();
  }

  void onDraw(al::Graphics& g) override {
    g.clear();
    al::gl::depthTesting(true);
    
    // Draw coordinate reference axes
    g.lineWidth(2.0);
    al::Mesh axes;
    axes.primitive(al::Mesh::LINES);
    
    // X axis (red)
    g.color(1, 0, 0);
    axes.vertex(0, 0, 0);
    axes.vertex(1, 0, 0);
    
    // Y axis (green)
    g.color(0, 1, 0);
    axes.vertex(0, 0, 0);
    axes.vertex(0, 1, 0);
    
    // Z axis (blue)
    g.color(0, 0, 1);
    axes.vertex(0, 0, 0);
    axes.vertex(0, 0, 1);
    
    g.draw(axes);
    mAudioManager.draw(g);
    mAudioManager.drawGUI(g);
    
  }

  void onSound(al::AudioIOData& io) override {
    mAudioManager.processAudio(io); // render audio
  }
  
  // Mouse event handling
  bool onMouseMove(const al::Mouse& m) override {
    mAudioManager.onMouseMove(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDown(const al::Mouse& m) override {
    mAudioManager.onMouseDown(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseDrag(const al::Mouse& m) override {
    mAudioManager.onMouseDrag(graphics(), m, width(), height());
    return true;
  }
  
  bool onMouseUp(const al::Mouse& m) override {
    mAudioManager.onMouseUp(graphics(), m, width(), height());
    return true;
  }
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.title("3D Sound Spatialization");
  app.configureAudio(AUDIO_CONFIG);
  app.start();
  return 0;
}