#ifndef PHOTON_H
#define PHOTON_H

struct Photon {
    int ls, rs, d;
    Vector3f power;
    Vector3f position;
    Vector3f direction;
    Vector3f absorb;
    double currentN;
    Photon() {
        ls = rs = d = 0;
        absorb = Vector3f(0);
        currentN = 1;
    }
    Photon(const Vector3f &po, const Vector3f &pos, const Vector3f &dir){
        power = po, position = pos, direction = dir;
        ls = rs = d = 0;
        absorb = Vector3f(0);
        currentN = 1;
    }
};

#endif //PHOTON_H