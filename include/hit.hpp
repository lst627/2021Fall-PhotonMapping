#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "ray.hpp"

class Material;

class Hit {
public:

    // constructors
    Hit() {
        material = nullptr;
        t = 1e38;
    }

    Hit(double _t, Material *m, const Vector3f &n) {
        t = _t;
        material = m;
        normal = n;
    }

    Hit(const Hit &h) {
        t = h.t;
        material = h.material;
        normal = h.normal;
    }

    // destructor
    ~Hit() = default;

    double getT() const {
        return t;
    }

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

    Material *getMaterial() const {
        return material;
    }

    const Vector3f &getNormal() const {
        return normal;
    }

    void set(double t, Material *m, const Vector3f &n, double x = 0, double y = 0) {
        this->t = t;
        this->material = m;
        this->normal = n;
        this->x = x;
        this->y = y;
    }

private:
    double t;
    Material *material;
    Vector3f normal;
    double x, y;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
