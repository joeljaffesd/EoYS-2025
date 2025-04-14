#ifndef ORBIT_HPP
#define ORBIT_HPP

#include "al/math/al_Vec.hpp"
#include "vfxUtility.hpp"

//a translation effect


class OrbitEffect : public VertexEffect {
public:
    float rate = 1.0f;                     // How fast the orbit moves (Hz)
    float radius = 1.0f;                   // How far from the orbit center
    al::Vec3f orbitCenter = {0, 0, 0};     // The point the whole mesh orbits around
    int rotationAxes = 1; //axis mesh rotates on. 1 = x, 2 = y, 3 = z, 4 = xy, 5 = xz, 6 = zy

    OrbitEffect(float r = 1.0f, float rad = 1.0f, al::Vec3f center = {0, 0, 0}, int axes = 1)
        : rate(r), radius(rad), orbitCenter(center), rotationAxes(axes){}

    void process(std::vector<al::Vec3f>& verts, float t) override {
        // Compute orbit offset based on time
        float angle = t * rate * M_2PI; // 2 pi makes it so a rate of 1 is a full cycle per second
        float x = std::cos(angle) * radius;
        float y =  std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        al::Vec3f orbitOffset;

        switch (rotationAxes) {
                case 1: orbitOffset =  {x, y, z}; break;
                //case 2: v.y += offset; break;
                //case 3: v.z += offset; break;
                default: break; // ignore invalid axis
            }



        // center pos is where the mesh should be in this given frame. using it to shift individual vertices in later loop
        al::Vec3f centerPosition = orbitCenter + orbitOffset;

        // Compute the current center of the mesh
        al::Vec3f meshCenter{0, 0, 0};
        for (auto& v : verts) {
            meshCenter += v; //summing all vertex position
        }
        meshCenter /= verts.size(); // average of all verts is the center of the object 

        // Compute translation -  move mesh center to orbit path
        al::Vec3f shift = centerPosition - meshCenter;

        // Move all vertices by the shift
        for (auto& v : verts) {
            v += shift;
        }
    }
};


#endif