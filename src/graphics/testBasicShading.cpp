#include "al/app/al_App.hpp"           // AlloLib app framework
#include "al/graphics/al_Shader.hpp"    // Access to ShaderProgram

#include <fstream>   // For reading shader files
#include <sstream>   // For loading entire file into string

using namespace al;

struct BasicShaderApp : App {
  ShaderProgram shader;    // Our shader program object
  Mesh mesh;               // Our fullscreen mesh (rectangle)

  // Helper function: read an entire text file into a std::string. this will read in our shader files 
  std::string loadFile(const std::string& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  void onCreate() override {
   

    // Fragment shader: load in my files
    std::string vertSource = loadFile("../assets/shaders/fullscreen.vert");
std::string fragSource = loadFile("../assets/shaders/static_color.frag");
    std::cout << "VERTEX SHADER:\n" << vertSource << "\n";
std::cout << "FRAGMENT SHADER:\n" << fragSource << "\n";

    shader.compile(vertSource, fragSource);

    mesh.primitive(Mesh::TRIANGLE_STRIP);
  mesh.vertex(Vec3f(-1.0f, -1.0f, 0.0f));
  mesh.vertex(Vec3f( 1.0f, -1.0f, 0.0f));
  mesh.vertex(Vec3f(-1.0f,  1.0f, 0.0f));
  mesh.vertex(Vec3f( 1.0f,  1.0f, 0.0f));



    // camera for testing
    // nav().pos(0, 0, 2);           
    // navControl().active(false); 

    //camera for debugging, will need to change   
    nav().pos(0, 0, 0);            // Set camera at the origin
nav().faceToward(Vec3d(0, 0, -1)); // Face down the negative Z axis
navControl().disable();    
  }

  void onDraw(Graphics& g) override {

    
    // === 5. Standard render loop ===
    g.clear(0);                     // Clear the screen (black)
    g.viewport(0, 0, width(), height()); 
    g.depthTesting(false);          // No depth needed for flat 2D

    shader.begin();                 // Activate the shader
    g.draw(mesh);                   // Draw the fullscreen rectangle
    shader.end();                   // Deactivate the shader
  }
};

// Standard AlloLib main entry point
int main() {
  BasicShaderApp().start();
}
