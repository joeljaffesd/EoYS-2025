#pragma once
#include "shadedMesh.hpp" 
#include "al/graphics/al_Shapes.hpp"


/*
uses shadedMesh to wrap a shader to a sphere -- EXAMPLE USAGE AT BOTTOM OF THIS FILE. ALSO IN sphereShaderExample.cpp

  M1 issue / warning
  - We MUST send a dynamic uniform every frame (typically a Mat4) 
    to prevent Metal driver from freezing uniform updates.
*/

struct ShaderToSphere {
    ShadedMesh shadedMesh;
    float radius = 1.0f;
    int subdivisions = 100; // smoothness of sphere
    float pointSize = 10.0f;

    /// Initialize shaders
    /// @param vertPath Path to vertex shader
    /// @param fragPath Path to fragment shader
    bool setShaders(const std::string& vertPath, const std::string& fragPath) {
        return shadedMesh.setShaders(vertPath, fragPath);
    }

    /// sphere vertices
    /// @param r sphere radius
    /// @param subdiv number of subdivisions
    void setSphere(float r, int subdiv = 100) {
        radius = r;
        subdivisions = subdiv;
        shadedMesh.mesh.reset();
        shadedMesh.mesh.primitive(al::Mesh::TRIANGLE_FAN);
        al::addTexSphere(shadedMesh.mesh, 15, 250, true);
        // shadedMesh.mesh.primitive(al::Mesh::POINTS);

        // for (int j = 0; j <= subdivisions; ++j) {
        //     float v = float(j) / subdivisions;
        //     float theta = v * M_PI;

        //     for (int i = 0; i <= subdivisions; ++i) {
        //         float u = float(i) / subdivisions;
        //         float phi = u * M_2PI;

        //         float x = radius * sin(theta) * cos(phi);
        //         float y = radius * cos(theta);
        //         float z = radius * sin(theta) * sin(phi);

        //         shadedMesh.mesh.vertex(al::Vec3f(x, y, z));
        //     }
        // }
        shadedMesh.mesh.update(); // 🔥 Push to GPU
    }

    /// Set uniforms
    void setUniformFloat(const std::string& name, float value) {
        shadedMesh.setUniformFloat(name, value);
    }

    /// Update view/projection matrices - should leave 
    void setMatrices(const al::Mat4f& view, const al::Mat4f& proj) {
        shadedMesh.setMatrices(view, proj);
    }

    /// Draw the sphere
    void draw(al::Graphics& g) {
        shadedMesh.shader.use();
        g.pointSize(pointSize);
        g.depthTesting(true);
        g.draw(shadedMesh.mesh);
    }
};



//EXAMPLE IMPLEMENTATION 

/*DECLARE: 
ShaderToSphere shaderSphere;
etc
void onCreate() override {
        shaderSphere.setShaders("../src/shaders/fullscreen.vert", "../src/shaders/static_color.frag");
        shaderSphere.setSphere(15.0f, 1000);

        etc
        }
etc

  void onDraw(Graphics& g) override {
        g.clear(0);
        shaderSphere.setMatrices(view().viewMatrix(), view().projMatrix(width(), height())); -- this is pretty boiler plate should leave
        shaderSphere.setUniformFloat("u_time", (float)t);
        //more uniforms etc
        shaderSphere.draw(g);

        etc
    }

*/
