#include "al/app/al_DistributedApp.hpp"
#include "../../graphics/videoToSphere.hpp"

class VideoApp : public DistributedApp {
public:
  VideoSphereLoader loader;

  virtual void onCreate() override {
    // Set the video file path
    loader.loadVideo("../assets/videos/vid.mp4");
  }

  virtual void onAnimate(al_sec dt) override {
    // Update the video sphere loader with the elapsed time
    loader.update(dt);
  }

  virtual bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {  // Spacebar toggles play/pause
      loader.togglePlayPause();
      std::cout << (loader.isPlaying() ? "Playing" : "Paused") << std::endl;
    } else if (k.key() == 'r') {  // 'r' to restart
      loader.restart();
      std::cout << "Restarted playback" << std::endl;
    } else if (k.key() == 'l') {  // 'l' to toggle looping
      loader.toggleLooping();
      std::cout << "Looping: " << (loader.isLooping() ? "ON" : "OFF") << std::endl;
    }
    return true;
  }

  virtual void onDraw(Graphics &g) override {
    g.clear(0.1f);
    loader.draw(g);
  }

};

int main() {
  VideoApp app;
  app.start();
  return 0;
}