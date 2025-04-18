#ifndef MANUAL_PULSE_EFFECT_HPP
#define MANUAL_PULSE_EFFECT_HPP

#include "al/math/al_Vec.hpp"
#include "vfxUtility.hpp" //

// wanted to use built in scale function, but caused issues with saving state
// and unwanted vertices updating

// a transformation effect
class ManualPulseEffect : public VertexEffect {
public:
  float amount = 0.2f; // amount of pulsing (expansion/contraction)
  int direction = 1;   // 1 = expand outward, -1 = collapse inward

  /**
   * @brief Set effect parameters
   *
   * @param Amount  Displacement strength (0.0–1.0)
   * @param Direction     Direction: 1 = expand, -1 = contract
   */
  void setParams(float Amount, int Direction) {

    amount = Amount;
    direction = Direction;
  }

  void process(al::VAOMesh &mesh, float t) override {
    float pulseScale = 1.0f + (amount * direction);
    float delta = mesh.vertices()[1].y - baseVerts[1].y;
    if (abs(delta) >= 2.0f) {
      mesh.vertices() = baseVerts;
      // mesh.scale(delta);
      // direction *= -1;
    }
    // }
    else {
      mesh.scale(pulseScale);
    }
    // auto &verts = mesh.vertices();
    // double delta;
    // delta = mesh.vertices[1].x - baseVerts[1];
    // if (verts.size() != baseVerts.size())
    //   return;
    // mesh.scale((1 + amount));

    // al::Vec3f center{0, 0, 0};
    //  for (auto &v : baseVerts)
    //    center += v;
    //  center /= baseVerts.size(); // center of whole mesh

    // for (size_t i = 0; i < verts.size(); ++i) {
    //  al::Vec3f fromCenter = baseVerts[i] - center;
    //   keeps effect relative to initial geometry, not last frame ( this
    //   would cause rapid feedback shrink / grow issue) verts[i] = center +
    //   fromCenter * (1.0f + pulseAmount); no frame to frame distortion
    //}

    mesh.update();
  }
};

#endif //
