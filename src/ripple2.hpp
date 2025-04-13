#ifndef RIPPLE2_HPP
#define RIPPLE2_HPP

#include "vfxEngine.hpp"

// a transformation effect
//working on more generic ripple effect
class RippleEffect : public VertexEffect {
public:
    float rate = 5.0f;
    float mix = 0.2f;
    float spatialFreq = 5.0f; // 5 spatial ripples across the object, might remove this later

    char axis = 'z';  // 'x', 'y', or 'z'

    RippleEffect(float freq = 5.0f, float amp = 0.2f, char ax = 'z', float waves = 5.0f)
        : rate(freq), mix(amp), axis(ax), spatialFreq(waves) {}

    void process(std::vector<al::Vec3f>& verts, float t) override {
        for (auto& v : verts) {
            float offset = std::sin(v.x * spatialFreq + t * rate * M_2PI) * (mix/10.f); //multiply spatial freq in sin

            switch (axis) {
                case 'x': v.x += offset; break;
                case 'y': v.y += offset; break;
                case 'z': v.z += offset; break;
                default: break; // ignore invalid axis
            }
        }
    }
};

#endif