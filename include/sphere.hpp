#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D {
public:

    Sphere() {
        // unit ball at the center
        this->center = Vector3f(0,0,0);
        this->radius = 1;
        this->radius2 = 1;
    }
    
    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        // 
        this->center = center;
        this->radius = radius;
        radius2 = radius*radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, double tmin) {
        //
        Vector3f l = center - r.getOrigin();
        float l_len2 = norm2(l);
        float t_p = Vector3f::dot(l, r.getDirection().normalized());
        if(t_p < 0 && l_len2 > radius2 + eps) 
            return false;
        float d2 = l_len2 - t_p*t_p;
        if(d2 > radius2) 
            return false;
        float td2 = radius2 - d2;

        float t;
        if(l_len2 > radius2 + eps) t = t_p - sqrt(td2);
        else t = t_p + sqrt(td2);
        if (t > h.getT() + eps || t < tmin - eps || t < 0)
            return false;

        Vector3f n = r.pointAtParameter(t) - center;
        n = n.normalized();
        if (l_len2 > radius2 + eps) 
            h.set(t, material, n, 0, 0);
        else h.set(t, material, -n, 0, 0);
        return true;
    }

protected:

    Vector3f center;
    float radius;
    float radius2;

};


#endif
