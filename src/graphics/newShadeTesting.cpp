#include "al/app/al_App.hpp"
#include "ShadedMesh.hpp" // Our modular class

using namespace al;

struct MyApp : App {
    ShadedMesh shadedSquare;
    double t = 0.0;

    // For sending an identity matrix to force updates (important for Metal optimization bug)
    Mat4f dummyMatrix;

    void onCreate() override {
        // Setup full screen square
        shadedSquare.mesh.primitive(Mesh::TRIANGLE_STRIP);
        shadedSquare.mesh.vertex(Vec3f(-1.0f, -1.0f, 0.0f));
        shadedSquare.mesh.vertex(Vec3f( 1.0f, -1.0f, 0.0f));
        shadedSquare.mesh.vertex(Vec3f(-1.0f,  1.0f, 0.0f));
        shadedSquare.mesh.vertex(Vec3f( 1.0f,  1.0f, 0.0f));

        // Load shaders
        shadedSquare.setShaders(
            "../assets/shaders/fullscreen.vert",
            "../assets/shaders/static_color.frag"
        );

        dummyMatrix.setIdentity(); // always ready

        nav().pos(0, 0, 0);
        nav().faceToward(Vec3d(0, 0, -1));
        navControl().disable();
    }

    void onAnimate(double dt) override {
        t += dt;
    }

    void onDraw(Graphics& g) override {
        g.clear(0);
        g.viewport(0, 0, width(), height());
        g.depthTesting(false);

        // --- Shader Drawing ---
        shadedSquare.shader.use();

        // Critical: Always pass a matrix uniform to prevent driver optimization!
        shadedSquare.setUniformMat4f("dummyModelview", dummyMatrix);

        // Set your intended dynamic uniform
        shadedSquare.setUniformFloat("u_time", (float)t);

        g.draw(shadedSquare.mesh);
    }
};

int main() {
    MyApp app;
    app.start();
}
