#ifndef ORBIT_EFFECT_HPP
#define ORBIT_EFFECT_HPP

#include "al/math/al_Vec.hpp"
#include "vfxUtility.hpp"  // base VertexEffect class

// a translation effect
//need to add inline comments 
/**
 * @brief Mesh translation effect for orbiting object around specific point.  
 * Effect must be created before onCreate (in app public section.)
 * Effect must be passed into a VFX Chain instance via effectChain.push back in on create
 * Set params is initialized in onCreate. can be updated in onAnimate.
 */
class OrbitEffect : public VertexEffect {
public:
    float rate = 1.0f;                // orbit speed in hz
    float radius = 1.0f;              //  distance from orbit center - in unit of 3d space. might change this
    al::Vec3f orbitCenter = {0, 0, 0}; // 

    int xDir = 1;  // direction (-1 or 1)
    int yDir = 1;  // direction (-1 or 1)
    int zDir = 1;  // direction (-1 or 1)

    int rotationAxes = 0; // 0 = XZ orbit, 1 = XY, 2 = YZ

    //constructor and initialize members
    OrbitEffect(float r = 1.0f,
                float rad = 1.0f,
                al::Vec3f center = {0, 0, 0},
                int axes = 0,
                int xD = 1,
                int yD = 1,
                int zD = 1)
        : rate(r),
          radius(rad),
          orbitCenter(center),
          xDir(xD),
          yDir(yD),
          zDir(zD),
          rotationAxes(axes) {}

    void process(al::VAOMesh& mesh, float t) override {
        //reference to vertices
        auto& verts = mesh.vertices(); 

        // Compute angular position on orbit path
        float angle = t * rate * M_2PI;  // 2π means 1Hz = one full loop per second

        // offset is specific to a plane 
        al::Vec3f orbitOffset{0, 0, 0};
        switch (rotationAxes) {
            case 0: // XZ orbit
                orbitOffset.x = std::cos(angle) * radius * xDir;
                orbitOffset.z = std::sin(angle) * radius * zDir;
                break;
            case 1: // XY orbit
                orbitOffset.x = std::cos(angle) * radius * xDir;
                orbitOffset.y = std::sin(angle) * radius * yDir;
                break;
            case 2: // YZ orbit
                orbitOffset.y = std::cos(angle) * radius * yDir;
                orbitOffset.z = std::sin(angle) * radius * zDir;
                break;
        }

        // target for entire mesh in a given frame
        al::Vec3f targetCenter = orbitCenter + orbitOffset;

        // calculate center of the whole mesh via average = (sum of all vertices / number of vertices)
        al::Vec3f meshCenter{0, 0, 0};
        for (const auto& v : verts) meshCenter += v;
        meshCenter /= verts.size();

        // shift amount for each vertex. 
        al::Vec3f shift = targetCenter - meshCenter;
        mesh.translate(shift); // use built-in translate for VAOMesh
    }
 /**
     * @brief Set effect parameters using named interface
     * 
     * @param rate           orbit frequency (Hz)
     * @param radius         orbit radius
     * @param orbitCenter    center point to orbit around
     * @param rotationAxes   plane of orbit (0 = XZ, 1 = XY, 2 = YZ)
     * @param xDir           direction along X (-1 or 1)
     * @param yDir           direction along Y (-1 or 1)
     * @param zDir           direction along Z (-1 or 1)
     */
    void setParams(float rate,
                   float radius,
                   al::Vec3f orbitCenter,
                   int rotationAxes = 0,
                   int xDir = 1,
                   int yDir = 1,
                   int zDir = 1) {
        this->rate = rate;
        this->radius = radius;
        this->orbitCenter = orbitCenter;
        this->rotationAxes = rotationAxes;
        this->xDir = xDir;
        this->yDir = yDir;
        this->zDir = zDir;
    }
};

#endif // ORBIT_EFFECT_HPP
