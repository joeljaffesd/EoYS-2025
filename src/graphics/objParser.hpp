#ifndef PARSE_OBJ_HPP
#define PARSE_OBJ_HPP

//might be using for parsing
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Vec.hpp"


class objParser {

    private:
    //useful is info needs to be stored not directly written to mesh
    std::vector<al::Vec3f> mVertices;
    std::vector<float> mNormals;
    std::vector<float> mLighting;

    public:

bool parse (std::string targetPath, al::Mesh& destinationMesh){
 std::ifstream file(targetPath);
    if (!file.is_open()) {
        std::cerr << "Couldn't open obj" << targetPath << std::endl;
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
    
    //// CONDITIONAL LOGIC FOR PARSING EACH DATA TYPE////

    //// VERTICES ////
    if (line.empty() || line[0] != 'v' || line[1] != ' ') {
      continue;
    }
    std::istringstream lineStream(line);
        std::string prefix;
        float x, y, z;

        lineStream >> prefix >> x >> y >> z;
        if (prefix == "v") {
            destinationMesh.vertex(al::Vec3f(x, y, z));
            //useful for storing vertices outside of mesh creation
            mVertices.push_back(al::Vec3f(x, y, z));
        }
    }

    //// LIGHTING  ////


    //NORMALS


    //ENDING LOGIC//
   file.close();

    std::cout << "OBJ file parsed. Loaded " << destinationMesh.vertices().size() << " vertices." << std::endl;
    return true;

}

    std::vector<al::Vec3f> getVertices(){
        return mVertices;
    }
    // do the same for the other data types
};


#endif