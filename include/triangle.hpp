#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
#include <algorithm>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle: public Object3D {

public:
	
	Vector3f normal;
	double d;
	Vector3f vertices[3];

	int textureVertex[3], normalVectorID[3];

	Triangle(Material* m) : Object3D(m) {}
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
		vertices[0] = a, vertices[1] = b, vertices[2] = c;
	}

	double det(Vector3f A, Vector3f B, Vector3f C) {
		return A.x() * (B.y()*C.z() - C.y()*B.z()) - B.x() * (A.y()*C.z() - C.y()*A.z()) + C.x() * (A.y()*B.z() - B.y()*A.z());
	}

	Vector3f check(Vector3f A, Vector3f B, Vector3f C, Vector3f Res, double a, double b, double c) {
		return a*A+b*B+c*C - Res;
	}

	bool intersect( const Ray& ray,  Hit& hit , double tmin) override {
		if(abs(Vector3f::dot(normal, ray.getDirection())) < eps) return false; //!!!!!!
        double t = (d - Vector3f::dot(normal, ray.getOrigin())) / Vector3f::dot(normal, ray.getDirection());
        if (t >= hit.getT() || t < tmin || t < 0)
            return false;
		
		Vector3f intersection = ray.pointAtParameter(t);
		Vector3f E0 = vertices[0] - intersection, E1 = vertices[1] - intersection, E2 = vertices[2] - intersection;
		Vector3f h1 = Vector3f::cross(E0,E1), h2 = Vector3f::cross(E1,E2), h3 = Vector3f::cross(E2,E0);
		Vector3f res(Vector3f::dot(h1,h2), Vector3f::dot(h2,h3), Vector3f::dot(h3,h1)); // !!!!!!

		// Vector3f chk = check(vertices[0], vertices[1], vertices[2], intersection, res.x(), res.y(), res.z());
		// if(norm(chk)>1e-3) {
		// 	printf("Error!");
		// 	printf("%.8f %.8f %.8f\n",chk.x(), chk.y(), chk.z());
		// }

		if (0 <= res.y() && 0 <= res.z() && 0 <= res.x()) {
			Vector3f n = normal;
			if (Vector3f::dot(ray.getDirection(), n) > 0)
				n = -n;
			hit.set(t, material, n, 0, 0);
			return true;
		}
        return false;
	}

	double MinCoord(int coord) {
		return min(vertices[0][coord], min(vertices[1][coord],vertices[2][coord]));
	}

	double MaxCoord(int coord) {
		return max(vertices[0][coord], max(vertices[1][coord],vertices[2][coord]));
	}

	void print() {
		for(int idx = 0; idx < 3 ; ++idx)
			vertices[idx].print();
		printf("\n");
	}

	void setpar(){
		Vector3f d1 = vertices[2] - vertices[0], d2 = vertices[1] - vertices[0];
		normal = Vector3f::cross(d1, d2);
		normal.normalize();
		d = Vector3f::dot(normal, vertices[0]);
		if (normal == Vector3f::ZERO)
			normal = Vector3f(0, 0, 1);
	}
	
protected:

};

#endif //TRIANGLE_H

