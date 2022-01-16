#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH, int isDOF, double lenRadius, int lenSampleNum, double focusDist) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
        this->isDOF = isDOF;
        this->lenRadius = lenRadius;
        this->lenSampleNum = lenSampleNum;
        this->focusDist = focusDist;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
    // parameters for DOF
    bool isDOF = false;
    double lenRadius;
    int lenSampleNum;
    double focusDist;
};

// TODO: Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, double angle, int isDOF, double lenRadius, int lenSampleNum, double focusDist) : Camera(center, direction, up, imgW, imgH, isDOF, lenRadius, lenSampleNum, focusDist) {
        // angle is in radian.
        this->angle = angle;
        this->dist = 0.5 * (double)imgH / tan (angle/2.0);
    }

    Ray generateRay(const Vector2f &point) override {
        // 
        Vector3f dRw = (point.x() - (double)(getWidth())/2.0) * horizontal + (point.y() - (double)(getHeight())/2.0) * up + dist * direction;
        dRw = dRw.normalized();
        return Ray(center, dRw);
    }

protected:
    double angle;
    double dist;
};

#endif //CAMERA_H
