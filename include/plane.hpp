#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() {
        
    }

    Plane(const Vector3f &normal, double d, Material *m) : Object3D(m) {
        this->d = -d;  // n dot x + d = 0
        n = normal;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, double tmin) override {
        if(abs(Vector3f::dot(n, r.getDirection().normalized())) < eps) return false;
        double t = -(d + Vector3f::dot(n,r.getOrigin())) / Vector3f::dot(n, r.getDirection().normalized());
        if (t > h.getT() + eps || t < tmin - eps)
            return false;
        Vector3f normal = n;
        normal = normal.normalized();
        if (Vector3f::dot(r.getDirection().normalized(), normal) > 0)
            normal = -normal;
        Vector3f p = r.pointAtParameter(t);
        Vector3f pX = Vector3f::cross(normal, material->textDirection).normalized(), pY = Vector3f::cross(normal, pX).normalized();
        double x = Vector3f::dot(p, pX);
        double y = Vector3f::dot(p, pY);
        h.set(t, material, normal, x, y);
        // printf("!!!!!\n");
        return true;
    }

protected:
    double d;
    Vector3f n;
};

#endif //PLANE_H
		


		

