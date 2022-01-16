#ifndef OBJECT3D_H
#define OBJECT3D_H
#define eps 1e-7
#include "ray.hpp"
#include "hit.hpp"
#include "material.hpp"

// Base class for all 3d entities.
class Object3D {
public:
    Object3D() : material(nullptr) {}

    virtual ~Object3D() = default;

    explicit Object3D(Material *material) {
        this->material = material;
    }

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, double tmin) = 0;
    double norm2(Vector3f v) {return v.x()*v.x() + v.y()*v.y() + v.z()*v.z();}
    double norm(Vector3f v) {return sqrt(v.x()*v.x() + v.y()*v.y() + v.z()*v.z());}
    Material *material;
    
protected:

};

#endif

