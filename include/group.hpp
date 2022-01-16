#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>


// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {

public:

    Group() {

    }

    explicit Group (int num_objects) {

    }

    ~Group() override {

    }

    bool intersect(const Ray &r, Hit &h, double tmin) override {
        bool flag = false;
        for (int i = 0; i < objects.size(); ++i) {
            flag |= objects[i]->intersect(r, h, tmin);
        }
        return flag;
    }

    void addObject(int index, Object3D *obj) {
        objects.push_back(obj);
    }

    int getGroupSize() {
        return objects.size();
    }

private:
    std::vector<Object3D*> objects;
};

#endif
	
