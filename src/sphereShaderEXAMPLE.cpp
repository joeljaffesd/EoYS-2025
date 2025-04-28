


// file for testing loading in shaders onto sphere

#include "al/app/al_App.hpp"
#include "src/graphics/shaderToSphere.hpp"

using namespace al;

struct MyApp : App {
    ShaderToSphere shaderSphere;
    double t = 0.0;

    void onCreate() override {
        shaderSphere.setShaders("../assets/shaders/fullscreen.vert", "../assets/shaders/static_color.frag");
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
        shaderSphere.setUniformFloat("u_time", (float)t);
        shaderSphere.draw(g);
    }
};

int main() {
    MyApp app;
    app.start();
}
