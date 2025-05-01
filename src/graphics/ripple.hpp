#ifndef RIPPLE2_HPP
#define RIPPLE2_HPP

#include "vfxUtility.hpp"

// a transformation effect
// currently redundant but need to prototype more effects before fixings
class RippleEffect : public VertexEffect {
public:
    // how fast the mesh oscillates
    float rate = 5.0f;

    // the displacement amount
    float mix = 0.2f;

    // axis to displace along: 'x', 'y', or 'z'
    char axis = 'z';

    // number of spatial subdivisions (ripples across the mesh)
    float spatialFreq = 5.0f;

    

    
    RippleEffect(float freq = 5.0f, float amp = 0.2f, char ax = 'z', float waves = 5.0f)
        : rate(freq), mix(amp), axis(ax), spatialFreq(waves) {}

    void process(al::VAOMesh& mesh, float t) override {
        auto& verts = mesh.vertices();
        for (auto& v : verts) {
            float offset = std::sin(v.x * spatialFreq + t * rate * M_2PI) * (mix / 10.0f);  // offset formula

            // displace along chosen axis
            switch (axis) {
                case 'x': v.x += offset; break;
                case 'y': v.y += offset; break;
                case 'z': v.z += offset; break;
                default: break;
            }
        }
    }

    /**
     * @brief Set effect parameters
     * 
     * @param freq         Oscillation frequency in Hz
     * @param amp          Maximum amplitude (scaled internally)
     * @param waves        Spatial frequency (number of ripples across mesh)
     * @param ax           Axis of displacement: 'x', 'y', or 'z'
     */
    void setParams(float freq, float amp, float waves, char ax) {
        rate = freq;
        mix = amp;
        spatialFreq = waves;
        axis = ax;
    }
};

#endif // RIPPLE2_HPP
