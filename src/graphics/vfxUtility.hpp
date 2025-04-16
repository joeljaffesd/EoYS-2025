#ifndef VFX_ENGINE_HPP
#define VFX_ENGINE_HPP

#include "al/graphics/al_VAOMesh.hpp"
#include <vector>
#include <cmath>

//updated

// want to make a more generic template class for any v effect that ovverides based on the mesh component the effect targets -- specifically: vertices (already implemented), normals, texture coords, etc (shading???)

//attempting to take runtime polymorphism approach like gimmel. might make some changes but is working for now
class VertexEffect {
public:
    virtual ~VertexEffect() = default;
    bool enabled = true;

    //overriden in actuall effect declaration
    virtual void process(al::VAOMesh& mesh, float t) = 0;

    //no longer using
     //virtual void setEffectParams(const std::vector<float>& params) {}
};

class VertexEffectChain {
public:
    void pushBack(VertexEffect* effect) {
        effects.push_back(effect);
    }
    //process gets called in every on animate (plug it into on animate)
    // verts is a reference, time gets passed in through onAnimate
    void process(al::VAOMesh& mesh, float t) {
        //loops through each effect in effects vector. i think this is working properly but need to test stacking multiple fx. e is a copy of a pointer basically
        //auto& verts = mesh.vertices(); // might change this to make it more modular. i.e. - set effect type
        for (auto* e : effects) {
            //check if effect is on, then run process. (e is a pointer)
            if (e->enabled) e->process(mesh, t); //process will be the actual effect logic
        }
        mesh.update(); //part of vao mesh

    }

private:
    std::vector<VertexEffect*> effects;
};
#endif
