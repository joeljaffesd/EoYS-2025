#ifndef AUTO_PULSE_EFFECT_HPP
#define AUTO_PULSE_EFFECT_HPP

#include "al/math/al_Vec.hpp"
#include "vfxUtility.hpp"  //

//wanted to use built in scale function, but cased issues with saving state and unwanted vertices updating

// a transformation effect
class AutoPulseEffect : public VertexEffect {
public:
    float rate = 1.0f;     // pulse frequency in Hz
    float amount = 0.2f;   // amount of pulsing (expansion/contraction)
    int direction = 1;     // 1 = expand outward, -1 = collapse inward

    // constructor and initialize members
    AutoPulseEffect(float r = 1.0f, float a = 0.2f, int d = 1)
        : rate(r), amount(a), direction(d) {}

    void setParams(float r, float a, int d = 1) {
        rate = r;
        amount = a;
        direction = d;
    }

    void process(al::VAOMesh& mesh, float t) override {
        auto& verts = mesh.vertices();

        // calculate a pulse value (oscillates around zero)
        float pulseAmount = std::sin(t * rate * M_2PI) * amount * direction;

        // get the mesh center
        al::Vec3f center{0, 0, 0};
        for (auto& v : verts) center += v;
        center /= verts.size();

        // apply pulse as displacement from center
        for (auto& v : verts) {
            al::Vec3f fromCenter = v - center;
            v += fromCenter.normalized() * pulseAmount;
        }

        mesh.update();
    }
};

#endif // PULSE_EFFECT_HPP
