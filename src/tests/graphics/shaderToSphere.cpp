#include "al/app/al_App.hpp"
#include "shadedMesh.hpp"
#include "al/graphics/al_Shapes.hpp"

using namespace al;

struct MyApp : App {
    ShadedMesh shadedSphere;
    double t = 0.0;

    void onCreate() override {
    // --- Set up the Sphere Mesh ---

    // Important reset: always reset VAOMesh first
    shadedSphere.mesh.reset();
    shadedSphere.mesh.primitive(Mesh::POINTS); // drawing as points (could swap later)

    // Build sphere manually. i was having issues with built in sphere generation. some llm help for sphere generation so i could focus on openGL issues.
    int latitudes = 1000;   // vertical subdivisions (higher = smoother sphere)
    int longitudes = 1000;  // horizontal subdivisions (higher = smoother sphere)
    float radius = 15.0f;   // size of sphere

    for (int j = 0; j <= latitudes; ++j) {
        float v = float(j) / latitudes;
        float theta = v * M_PI; // polar angle from north pole to south pole [0, PI]

        for (int i = 0; i <= longitudes; ++i) {
            float u = float(i) / longitudes;
            float phi = u * M_2PI; // azimuthal angle around equator [0, 2PI]

            // Manual spherical → Cartesian mapping
            float x = radius * sin(theta) * cos(phi);
            float y = radius * cos(theta);
            float z = radius * sin(theta) * sin(phi);

            shadedSphere.mesh.vertex(Vec3f(x, y, z)); // add vertex
        }
    }

    // issue fix - on M1/M2 Mac: must call .update() after building mesh**
    // If you don't, the mesh will not be sent to GPU and disappear instantly. 
    shadedSphere.mesh.update(); 

    // Set shaders - method from shadedMesh header
    shadedSphere.setShaders(
        "../src/shaders/fullscreen.vert",
        "../src/shaders/static_color.frag"
    );

    // camera
    nav().pos(0, 0, 6); 
    navControl().active(true);
}

    void onAnimate(double dt) override {
        t += dt; //using for uni
    }

    void onDraw(Graphics& g) override {
       
        g.clear(0); 
        // g.viewport(0, 0, width(), height()); // viewport is default
        g.depthTesting(true); // useful for 3d testing

        shadedSphere.shader.use(); // ACTIVATES SHADER. need to do this befoer setting uniforms. method comes from allolib shader program

        // FOR APPLE METAL DRIVERS (m1 mac issue)
        // must send a dynamic mat4 every frame (like view or projection matrix)
        // otherwise the GPU "optimizes out" uniforms like u_time and ignores updates - resulting in static shader.
        shadedSphere.setMatrices(view().viewMatrix(), view().projMatrix(width(), height()));

        // uniform
        shadedSphere.setUniformFloat("u_time", (float)t);

        // draw mesh sphere
        g.pointSize(10.0f); // 
        g.draw(shadedSphere.mesh);
    }
};

int main() {
    MyApp app;
    app.start();
}
