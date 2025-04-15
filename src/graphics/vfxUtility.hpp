#ifndef VFX_ENGINE_HPP
#define VFX_ENGINE_HPP

#include "al/graphics/al_Mesh.hpp"
#include <vector>
#include <cmath>


// want to make a more generic template class for any v effect that ovverides based on the mesh component the effect targets -- specifically: vertices (already implemented), normals, texture coords, etc (shading???)

//attempting to take runtime polymorphism approach like gimmel. might make some changes but is working for now
class VertexEffect {
public:
    virtual ~VertexEffect() = default;
    bool enabled = true;

    virtual void process(std::vector<al::Vec3f>& verts, float t) = 0;
};

class VertexEffectChain {
public:
    void pushBack(VertexEffect* effect) {
        effects.push_back(effect);
    }
    //process gets called in every on animate (plug it into on animate)
    // verts is a reference, time gets passed in through onAnimate
    void process(std::vector<al::Vec3f>& verts, float t) {
        //loops through each effect in effects vector. i think this is working properly but need to test stacking multiple fx. e is a copy of a pointer basically
        for (auto* e : effects) {
            //check if effect is on, then run process. (e is a r)
            if (e->enabled) e->process(verts, t);
        }
    }

private:
    std::vector<VertexEffect*> effects;
};
#endif
