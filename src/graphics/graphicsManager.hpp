#ifndef EOYS_GRAPHICS_MANAGER
#define EOYS_GRAPHICS_MANAGER

#include "imageToSphere.hpp"
#include "assetEngine.hpp"
#include "shaderEngine.hpp"
#include "videoToSphereCV.hpp"

/**
 * @brief Graphics Manager handles scene transitions and maintains a collection of GraphicsVoice objects
 * 
 * This class manages a window of scenes (previous, current, next) and provides smooth
 * animations when transitioning between scenes. It efficiently loads and unloads scenes
 * as needed and provides methods to navigate between scenes.
 */
class GraphicsManager {
private:
  al::ParameterInt index { "Index", "", 0, 0, std::numeric_limits<int>::max() };
  al::ParameterBool updateFlag { "updateFlag", "", true };
  bool localUpdateFlag = false;
  std::vector<std::function<void()>> mCallbacks;
  std::vector<GraphicsVoice*> mGraphicsVoices;
  al::ParameterServer* mParameterServer;
  al::ParameterBool animationFlag { "animationFlag", "", false };
  bool localAnimationFlag = false;
  al::ParameterInt transitionDirectionParam { "transitionDirection", "", 1, -1, 1 };
  int localTransitionDirection = 1;
  
  // Animation state
  unsigned frameCounter = 0;
  const unsigned animationDuration = 300; // Animation duration in frames
  float animationOffset = 0.0f;        // Current animation vertical offset
  bool isAnimating = false;            // Whether an animation is in progress
  int currentSceneIndex = 0;           // Index of current scene in mGraphicsVoices
  int prevSceneIndex = -1;             // Index of previous scene (for removal)
  
  // Scene transition direction (1 for forward, -1 for backward)
  int transitionDirection = 1;

  // Transition state
  enum TransitionState {
    IDLE,             // No transition happening
    TRANSITION_IN,    // New scene is sliding in
    TRANSITION_OUT    // Old scene is sliding out
  };
  
  TransitionState currentTransition = IDLE;

public:

  /**
   * @brief Called during animation to update frame counter and manage scene transitions
   */
  void animationCallback() {
    if (!isAnimating) return;
    
    frameCounter++;
    
    // Calculate smooth animation offset (ease-in/ease-out)
    float progress = float(frameCounter) / animationDuration;
    
    // Simple ease-in/ease-out function: y = 3*x^2 - 2*x^3
    float easedProgress = 3.0f * progress * progress - 2.0f * progress * progress * progress;
    
    // Scale based on direction (use the synchronized direction)
    animationOffset = transitionDirection * easedProgress * 30.0f;
    
    // Handle transition completion
    if (frameCounter >= animationDuration) {
      // Reset animation state
      frameCounter = 0;
      isAnimating = false;
      animationOffset = 0.0f;
      localAnimationFlag = !localAnimationFlag;
      
      // Clean up if needed
      if (prevSceneIndex >= 0 && mGraphicsVoices.size() > 1) {
        try {
          // Only proceed if prevSceneIndex is valid
          if (prevSceneIndex < mGraphicsVoices.size()) {
            // Remove the old scene (prevSceneIndex) from mGraphicsVoices
            GraphicsVoice* voiceToRemove = mGraphicsVoices[prevSceneIndex];
            mGraphicsVoices.erase(mGraphicsVoices.begin() + prevSceneIndex);
            delete voiceToRemove;
            
            // Update current scene index if necessary
            if (prevSceneIndex < currentSceneIndex) {
              currentSceneIndex--;
            }
          }
        } catch (const std::exception& e) {
          std::cerr << "Exception during scene cleanup: " << e.what() << std::endl;
        }
        prevSceneIndex = -1;
      }
      
      currentTransition = IDLE;
    }
  }

  /**
   * @brief Prepares a new GraphicsVoice instance of the specified type
   * 
   * @tparam TGraphicsVoice The type of GraphicsVoice to create
   * @return TGraphicsVoice* Pointer to the newly created GraphicsVoice
   */
  template <class TGraphicsVoice>
  TGraphicsVoice* prepareVoice() {
    auto* voice = new TGraphicsVoice();
    voice->init();
    return voice;
  }
  
  /**
   * @brief Registers a GraphicsVoice with the manager and adds it to the scene
   * 
   * @param voice Pointer to the GraphicsVoice to register
   */
  void registerVoice(GraphicsVoice* voice) {
    try {
      if (!voice) {
        std::cerr << "Attempted to register null GraphicsVoice" << std::endl;
        return;
      }
      
      // Register parameters
      mParameterServer->registerParameterBundle(voice->params());
      
      // Start transition
      isAnimating = true;
      frameCounter = 0;
      currentTransition = TRANSITION_IN;
      
      // Store previous scene index for cleanup
      if (!mGraphicsVoices.empty()) {
        prevSceneIndex = currentSceneIndex;
      }
      
      // Add new voice to the collection
      mGraphicsVoices.push_back(voice);
      currentSceneIndex = mGraphicsVoices.size() - 1;
      
      // Trigger animation
      animationFlag = !animationFlag;
    } catch (const std::length_error& e) {
      std::cerr << "String length error during voice registration: " << e.what() << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "Exception during voice registration: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown exception during voice registration" << std::endl;
    }
  }

  GraphicsManager() {
    // callback on index change
    this->index.registerChangeCallback([this](int value){ 
      std::cout << "Index at: " << value << std::endl;
    });

    mCallbacks.push_back([this]() {
      auto* mImageLoader = this->prepareVoice<ImageSphereLoader>();
      mImageLoader->loadImage("../assets/images/imgWrap.png");
      this->registerVoice(mImageLoader);
    });

    mCallbacks.push_back([this]() {
      auto* mAssetEngine = this->prepareVoice<AssetEngine>();
      mAssetEngine->loadAsset("../assets/3dModels/eye/eye.obj",
       "../assets/3dModels/eye/eye.png");
      this->registerVoice(mAssetEngine);
    });

    mCallbacks.push_back([this]() {
      auto* mShaderEngine = this->prepareVoice<ShaderEngine>();
      mShaderEngine->shader("../src/shaders/Reactive-shaders/fractal1.frag");
      this->registerVoice(mShaderEngine);
    });

    mCallbacks.push_back([this]() {
      auto* mVideoLoader = this->prepareVoice<VideoSphereLoaderCV>();
      mVideoLoader->loadVideo("../assets/videos/vid.mp4");
      this->registerVoice(mVideoLoader);
    });

    mCallbacks.push_back([this]() {
      auto* mVideoLoader = this->prepareVoice<VideoSphereLoaderCV>();
      mVideoLoader->loadVideo("../assets/scenes/manInTheBox/inTheBox.mp4");
      this->registerVoice(mVideoLoader);
    });

    mCallbacks.push_back([this]() {
      auto* mVideoLoader = this->prepareVoice<VideoSphereLoaderCV>();
      mVideoLoader->loadVideo("../assets/scenes/manInTheBox/homunculusBG.mp4");
      this->registerVoice(mVideoLoader);
    });

  }

  ~GraphicsManager() {
    // Clean up all GraphicsVoice objects
    for (auto* voice : mGraphicsVoices) {
      delete voice;
    }
    mGraphicsVoices.clear();
  }

  void registerParameters(al::ParameterServer& server) {
    server.registerParameter(index);
    server.registerParameter(updateFlag);
    server.registerParameter(animationFlag);
    server.registerParameter(transitionDirectionParam);
    this->mParameterServer = &server;
  }

  /**
   * @brief Initialize the graphics manager
   * 
   * @param isReplica Whether this is a replica instance
   */
  void init(bool isReplica = false) {
    // Initial state setup
    currentSceneIndex = 0;
    prevSceneIndex = -1;
    isAnimating = false;
    frameCounter = 0;
    animationOffset = 0.0f;
    currentTransition = IDLE;
  }

  /**
   * @brief Update the graphics manager and all active voices
   * 
   * @param dt Delta time since last update
   */
  void update(double dt = 0) {
    // Check if we've reached the end of the available scenes
    if (index >= mCallbacks.size()) {
      // Go back to the previous scene index
      this->index = index - 1;
      updateFlag = !updateFlag;
    }

    // Handle scene update requests
    if (updateFlag != localUpdateFlag) {
      localUpdateFlag = !localUpdateFlag;
      try {
        if (index < mCallbacks.size()) {
          mCallbacks[index]();
        } else {
          std::cerr << "Warning: Index " << index << " out of bounds for mCallbacks size " << mCallbacks.size() << std::endl;
        }
      } catch (const std::exception& e) {
        std::cerr << "Exception during scene update: " << e.what() << std::endl;
      }
    }

    // Handle animation updates
    if (animationFlag != localAnimationFlag) {
      this->animationCallback();
    }

    // Sync transition direction from network parameter
    if (transitionDirectionParam != localTransitionDirection) {
      localTransitionDirection = transitionDirectionParam;
      transitionDirection = localTransitionDirection;
    }

    // Update all active voices
    for (auto& voice : mGraphicsVoices) {
      voice->update(dt);
    }
  }

  /**
   * @brief Render the active scenes with appropriate transitions
   * 
   * @param g Graphics context to render to
   */
  void render(al::Graphics& g) {
    if (mGraphicsVoices.empty()) return;
    
    g.pushMatrix();
    
    // Always render the current scene
    if (currentSceneIndex >= 0 && currentSceneIndex < mGraphicsVoices.size()) {
      try {
        if (isAnimating) {
          // During animation, adjust the position based on transition direction and state
          if (currentTransition == TRANSITION_IN) {
            // New scene is sliding in from below
            g.pushMatrix();
            g.translate(0, -animationOffset + transitionDirection * 30.0f, 0);
            mGraphicsVoices[currentSceneIndex]->onProcess(g);
            g.popMatrix();
            
            // Old scene is sliding up and out
            if (prevSceneIndex >= 0 && prevSceneIndex < mGraphicsVoices.size()) {
              g.pushMatrix();
              g.translate(0, -animationOffset, 0);
              mGraphicsVoices[prevSceneIndex]->onProcess(g);
              g.popMatrix();
            }
          }
        } else {
          // Normal rendering when not animating
          mGraphicsVoices[currentSceneIndex]->onProcess(g);
        }
      } catch (const std::exception& e) {
        std::cerr << "Exception during rendering: " << e.what() << std::endl;
      } catch (...) {
        std::cerr << "Unknown exception during rendering" << std::endl;
      }
    }
    
    g.popMatrix();
  }

  /**
   * @brief Process audio for all active voices
   * 
   * @param io Audio IO data
   */
  void render(al::AudioIOData& io) {
    for (auto& voice : mGraphicsVoices) {
      voice->onProcess(io);
    }
  }

  /**
   * @brief Move to the next scene
   */
  void nextScene() {
    transitionDirection = 1; // Moving forward
    localTransitionDirection = 1;
    transitionDirectionParam = 1; // Sync with network
    this->index = index + 1;
    updateFlag = !updateFlag;
  } 

  /**
   * @brief Move to the previous scene
   */
  void prevScene() {
    transitionDirection = -1; // Moving backward
    localTransitionDirection = -1;
    transitionDirectionParam = -1; // Sync with network
    this->index = index - 1;
    updateFlag = !updateFlag;
  } 

};

#endif // EOYS_GRAPHICS_MANAGER