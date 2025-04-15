#ifndef EOYS_GRAPHICS_HPP
#define EOYS_GRAPHICS_HPP

#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Mesh.hpp"

/**
 * @brief TODO
 */
class GraphicsEngine {
public:
  GraphicsEngine() {}
  void addImage() {} // TODO
  void render(al::Graphics& g) {
    g.clear(0);
    g.pushMatrix();
    // Draw here
    g.popMatrix();
  }
};

#endif