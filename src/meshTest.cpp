#include "al/app/al_App.hpp"
#include <iostream>
#include <cmath>
#include "al/graphics/al_VAOMesh.hpp"
#include "vfxUtility.hpp" // 
#include "ripple2.hpp"
#include "orbit.hpp"

using namespace al;

// this is a mesh useful for testing effects. the mesh is pretty much static until a time-based vertex effect is applied

class MyApp : public App {
public:
    //Mesh mesh;
    VAOMesh mesh;
    VAOMesh mesh2;
    float t = 0.0f;

    int gridSize = 20;
    float spacing = 0.1f;

    RippleEffect ripple; 
    RippleEffect ripple2;  
    OrbitEffect orbit;
    OrbitEffect orbit2;
    VertexEffectChain effectChain; // chain constructed
    VertexEffectChain effectChain2;

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
                    mesh2.vertex(Vec3f(
                        x /2 *  spacing,
                        y / 2* spacing,
                        z / 2* spacing
                    
                    ));
                    
                }
            }
        }

        mesh.color(1, 1, 1);
        mesh2.color(0.6, 1, 1);

        // Set effect parameters if desired
        ripple.setParams(1.0, 0.2, 4.0, 'y');
        
        orbit.setParams(1.0, 1.0, {0,2,1}, 0, -1, 1, 1);         
        // push effects to chain
        effectChain.pushBack(&ripple); 
        effectChain.pushBack(&orbit);

        /////// END MESH 1 EFFECTS //////

        //// START MESH 2 EFFECTS/////
        orbit2.rate = 0.9f;
        orbit2.radius = 0.9f;
        orbit2.rotationAxes = 1;
        orbit2.orbitCenter = {0,2,0};
        orbit2.xDir = -1;
        orbit2.yDir = -1;
        orbit2.zDir = 1;
        effectChain2.pushBack(&orbit2);


    }

    void onAnimate(double dt) override {
        t += dt;

        effectChain.process(mesh, t); // run process function on chain
        effectChain2.process(mesh2, t);
    }

    void onDraw(Graphics& g) override {
        g.clear(0.1);
        g.pointSize(2.0);
        g.color(0.5, 0.5, 1.0);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        g.draw(mesh);
        g.draw(mesh2);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
};

int main() {
    MyApp app;
    app.start();
}
