#include "al/app/al_App.hpp"
#include <iostream>
#include <cmath>
#include "vfxEngine.hpp" // 
#include "ripple2.hpp"

using namespace al;

// this is a mesh useful for testing effects. the mesh is pretty much static until a time-based vertex effect is applied

class MyApp : public App {
public:
    Mesh mesh;
    float t = 0.0f;

    int gridSize = 20;
    float spacing = 0.1f;

    RippleEffect ripple;           // effect constructed
    VertexEffectChain effectChain; // chain constructed

    void onCreate() override {
        nav().pos(0, 0, 3);
        mesh.primitive(Mesh::POINTS);
        //mesh.primitive(Mesh::TRIANGLES);
        //mesh.primitive(Mesh::TRIANGLE_FAN);

        for (int x = -gridSize; x <= gridSize; ++x) {
            for (int y = -gridSize; y <= gridSize; ++y) {
                for (int z = -gridSize; z <= gridSize; ++z) {
                    mesh.vertex(Vec3f(
                        x * spacing,
                        y * spacing,
                        z * spacing
                    ));
                }
            }
        }

        mesh.color(1, 1, 1);

        // Set effect parameters if desired

        //inear wave settings//
        // ripple.frequency = 1000000.f;
        // ripple.amplitude = 30.f;

        // freq is in hz so i is one cycle per sec
        ripple.rate = 1.f;
        //0.1 is really high
        ripple.mix = 0.2f;
        ripple.axis = 'y'; //x, y, z chars
        ripple.spatialFreq = 3.0f; //basically number of waves


        // Add to chain
        effectChain.pushBack(&ripple); //pushing my effect to the chain
    }

    void onAnimate(double dt) override {
        t += dt;

        effectChain.process(mesh.vertices(), t); // run process function on chain
    }

    void onDraw(Graphics& g) override {
        g.clear(0.1);
        g.pointSize(2.0);
        g.color(0.5, 0.5, 1.0);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        g.draw(mesh);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
};

int main() {
    MyApp app;
    app.start();
}
