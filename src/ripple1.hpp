#ifndef RIPPLE1_HPP
#define RIPPLE1_HPP

#include "vfxEngine.hpp"

//this is an older test effect
// i like this ripple but its more of a specific effect than a template

class RippleEffect : public VertexEffect {
public:
    float frequency = 1.0f;
    float amplitude = 0.f;


    RippleEffect(float freq = 5.0f, float amp = 0.2f)
        : frequency(freq), amplitude(amp) {}

    void process(std::vector<al::Vec3f>& verts, float t) override {
        for (auto& v : verts) {
            v.z = std::sin(v.x * (frequency*60) + t*3) * amplitude;
        }
    }
};

#endif