#pragma once
#include "shadedMesh.hpp" 
#include "al/graphics/al_Shapes.hpp"


/*
uses shadedMesh to wrap a shader to a sphere -- EXAMPLE USAGE AT BOTTOM OF THIS FILE. ALSO IN sphereShaderExample.cpp

  M1 issue / warning
  - We MUST send a dynamic uniform every frame (typically a Mat4) 
    to prevent Metal driver from freezing uniform updates.
*/

class ShadedSphere : public ShadedMesh {
private:
  float radius = 1.0f;
  int subdivisions = 100; // smoothness of sphere
  float pointSize = 10.0f;

public:
  /// Initialize shaders
  /// @param vertPath Path to vertex shader
  /// @param fragPath Path to fragment shader

  /// sphere vertices
  /// @param r sphere radius
  /// @param subdiv number of subdivisions
  void setSphere(float r, int subdiv = 100) {
    radius = r;
    subdivisions = subdiv;
    this->reset();
    this->primitive(al::Mesh::TRIANGLE_FAN);
    al::addTexSphere(*this, 15, 24, true);
    // this->update(); // 🔥 Push to GPU // seg faults, move to draw
  }

  /// Update view/projection matrices - should leave 
  void setMatrices(const al::Mat4f& view, const al::Mat4f& proj) {
    this->setMatrices(view, proj);
  }

  bool first = true;
  /// Draw the sphere
  void draw(al::Graphics& g) {
    if (first) {
      this->update();
      first = false;
    }
    // this->update();
    this->mShader.use();
    g.pointSize(pointSize);
    // g.depthTesting(true);
    g.draw(*this);
    // g.depthTesting(false);
  }
};