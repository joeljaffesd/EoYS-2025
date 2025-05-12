#ifndef AUTO_PULSE_EFFECT_HPP
#define AUTO_PULSE_EFFECT_HPP

#include "al/math/al_Vec.hpp"
#include "vfxUtility.hpp"  //

//wanted to use built in scale function, but caused issues with saving state and unwanted vertices updating

/**
 * @brief Mesh effect scales entire mesh inward and outward at pulse rate. 
 * Must use .setBaseMesh in onCreate to create a reference point for scaling. Can be updated in onAnimate
 * Effect must be passed into a VFX Chain instance via effectChain.push back in on create
 * Set params is initialized in onCreate. can be updated in onAnimate.
 */
// a transformation effect
class AutoPulseEffect : public VertexEffect {
public:
    float rate = 1.0f;     // pulse frequency in Hz
    float amount = 0.2f;   // amount of pulsing (expansion/contraction)
    int direction = 1;     // 1 = expand outward, -1 = collapse inward

    /**
     * @brief Set effect parameters
     * 
     * @param Rate     Pulse rate in Hz
     * @param Amount  Displacement strength (0.0–1.0)
     * @param Direction     Direction: 1 = expand, -1 = contract
     */
    void setParams(float Rate, float Amount, int Direction) {
        rate = Rate;
        amount = Amount;
        direction = Direction;
    }


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

#endif // AUTO_PULSE_EFFECT_HPP
