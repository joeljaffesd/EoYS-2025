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

    // Original mesh state. need this for knowing where to pulse back to and handling additional transformation effects
    //std::vector<al::Vec3f> baseVerts;

    AutoPulseEffect(float r = 1.0f, float a = 0.3f, int d = 1)
        : rate(r), amount(a), direction(d) {}

    // Set the base mesh (e.g., from mesh.vertices()) //effect will not work without setting base mesh ( -> approach)
    // void setBaseMesh(const std::vector<al::Vec3f>& verts) {
    //     baseVerts = verts;
    // }

    void process(al::VAOMesh& mesh, float t) override {
        auto& verts = mesh.vertices();
        if (verts.size() != baseVerts.size()) return;

        float pulseAmount = std::sin(t * rate * M_2PI) * amount * direction; //oscillating pulse effect math

        al::Vec3f center{0, 0, 0};
        for (auto& v : baseVerts) center += v;
        center /= baseVerts.size(); //center of whole mesh 

        for (size_t i = 0; i < verts.size(); ++i) {
            al::Vec3f fromCenter = baseVerts[i] - center;
            //keeps effect relative to initial geometry, not last frame ( this would cause rapid feedback shrink / grow issue)
            verts[i] = center + fromCenter * (1.0f + pulseAmount);
            //no frame to frame distortion
        }



        mesh.update();
    }
};

#endif // PULSE_EFFECT_HPP
