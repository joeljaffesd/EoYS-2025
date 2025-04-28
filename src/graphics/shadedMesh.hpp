#pragma once // Only used once during compiling

#include "al/graphics/al_Shader.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Graphics.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

/*

  Trying to take a more modular approach:
  - This class allows for constructing multiple instances of shaded meshes
    in our main program, allowing easy shader -> mesh creation.
  - Goal: set shader asset files, mesh target, and uniforms.

  ***IMPORTANT for Apple M1 / Metal drivers ***
  - We MUST always send a dynamic uniform (like a mat4) each frame 
    to prevent the GPU from optimizing the shader as "static."
  - If you don't send a matrix, other uniforms (like u_time) will NOT update visually. This was the only solution i have found so far. I am not sure if it negatively impacts performance in a significant way. This may have to be tested in the future.
  - See onDraw() call order:
      1. .use()
      2. .setUniform(s)
      3. g.draw(mesh)
*/

class ShadedMesh {
public:
    al::ShaderProgram shader;
    al::VAOMesh mesh; // Will update to VAOMesh once working


    // Constructor
    ShadedMesh() {}

    // Load and set shaders. bool so we can immediately know if it worked.
    bool setShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    // Uniform setters (will add overloads as needed)
    void setUniformFloat(const std::string& name, float value);
    void setUniformInt(const std::string& name, int value);
    void setUniformVec3f(const std::string& name, const al::Vec3f& vec);
    void setUniformMat4f(const std::string& name, const al::Mat4f& mat);

    //updating for spherical purposes. not sure if this will work
    void setMatrices(const al::Mat4f& view, const al::Mat4f& proj);


private:
    // Helper function to load shader source code
    static std::string loadFile(const std::string& filePath);
};

//INLINE DEFS BELOW TO KEEP THINGS TIDY AND EFFICIENT. (there might be a better way to do this, its new to me)

// Load a shader source file into a string
inline std::string ShadedMesh::loadFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Shader Error: Cannot open file " << filePath << "\n";
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf(); // Slurp contents into memory
    return buffer.str();    // Convert to usable string
}

// Set vertex and fragment shaders from file paths
inline bool ShadedMesh::setShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    std::string vertexSource = loadFile(vertexShaderPath);
    std::string fragmentSource = loadFile(fragmentShaderPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "ShaderMesh Error: Shader source file empty.\n";
        return false;
    }

    if (!shader.compile(vertexSource, fragmentSource)) {
        std::cerr << "ShaderMesh Error: Shader failed to compile.\n";
        shader.printLog();
        return false;
    }

    std::cout << "ShaderMesh: Shaders compiled successfully.\n";
    return true;
}

//INLINE FUNCTIONS BELOW FOR SETTING UNIFORMS
// Set a single float uniform
/// @param name The uniform name inside the shader
/// @param value The float value to send
inline void ShadedMesh::setUniformFloat(const std::string& name, float value) {
    shader.use(); // 🚨 Make sure shader is active
    int loc = shader.getUniformLocation(name.c_str());
    if (loc >= 0) {
        shader.uniform(loc, value);
    } else {
        static std::unordered_map<std::string, bool> warned;
        if (warned.find(name) == warned.end()) {
            std::cerr << "[Warning] Uniform '" << name << "' not found in shader.\n";
            warned[name] = true;
        }
    }
}


// Set a single int uniform
/// @param name The uniform name inside the shader
/// @param value The int value to send
inline void ShadedMesh::setUniformInt(const std::string& name, int value) {
    shader.use();
    shader.uniform(name.c_str(), value);
}

// Set a vec3 uniform (3 floats: x, y, z)
/// @param name The uniform name inside the shader
/// @param value (x, y, z)
inline void ShadedMesh::setUniformVec3f(const std::string& name, const al::Vec3f& vec) {
    shader.use();
    shader.uniform(name.c_str(), vec);
}

// Set a mat4 uniform (4x4 matrix)
/// @param name The uniform name inside the shader
/// @param value 4x4 matrix
inline void ShadedMesh::setUniformMat4f(const std::string& name, const al::Mat4f& mat) {
    shader.use();
    shader.uniform(name.c_str(), mat);
}

//updating for spherical purposes, not sure if this will workl:
inline void ShadedMesh::setMatrices(const al::Mat4f& view, const al::Mat4f& proj) {
    shader.use(); // 🔥 Important: bind before setting uniforms
    shader.uniform("al_ModelViewMatrix", view);
    shader.uniform("al_ProjectionMatrix", proj);
}