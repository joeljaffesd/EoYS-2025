


// file for testing loading in shaders onto sphere

#include "al/app/al_App.hpp"
#include "src/graphics/shaderToSphere.hpp"

using namespace al;

struct MyApp : App {
    ShaderToSphere shaderSphere;
    double t = 0.0;

    void onCreate() override {
        // *** SET SHADER PATH HERE *** //
        // for shaders folder 1: 
        shaderSphere.setShaders("../assets/shaders/Moving-shaders/moving.vert", "../assets/shaders/Moving-shaders/moving2.frag");
        // for shader folder 2:
        //shaderSphere.setShaders("../assets/shaders/Shader-2/shaderToyDefault.vert", "../assets/shaders/Shader-2/shader2.frag");
        // *** END SET SHADER PATH *** //
        shaderSphere.setSphere(15.0f, 1000);

        nav().pos(0, 0, 6);
        navControl().active(true);
    }

    void onAnimate(double dt) override {
        t += dt;
    }

    void onDraw(Graphics& g) override {
        g.clear(0);
        shaderSphere.setMatrices(view().viewMatrix(), view().projMatrix(width(), height()));
        //doing this to handle uniforms being stuck . didnt need for example 1
        shaderSphere.shadedMesh.shader.use();


        //shaderSphere.shadedMesh.use(); //not properly accessing this member
        //For shader 1:
        shaderSphere.setUniformFloat("u_time", (float)t);
        //For shader 2:
        //shaderSphere.setUniformFloat("iTime", t);
        //shaderSphere.setUniformVec3f("iResolution", Vec3f(width(), (height()), 0.0f)); // this simulates values of a screen size. it gets passed into the vertex shader then converted to normalized spherical coords. should maybe change where this operation happens. 


        shaderSphere.draw(g);
    }
};

int main() {
    MyApp app;
    app.start();
}
