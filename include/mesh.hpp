#ifndef MESH_H
#define MESH_H

#include <vector>
#include "object3d.hpp"
#include "triangle.hpp"
#include "Vector2f.h"
#include "Vector3f.h"
#include <map>
#include <algorithm>

class BoundBox {
public:
	Vector3f minPos, maxPos;
	BoundBox(){
		minPos = Vector3f(INFINITY, INFINITY, INFINITY);
		maxPos = Vector3f(-INFINITY, -INFINITY, -INFINITY);
	}
    double GetMin3(double x, double y, double z) {return std::min(x,std::min(y,z));}
    double GetMax3(double x, double y, double z) {return std::max(x,std::max(y,z));}
	void UpdateBox(Triangle* tri){
        double mn, mx;
		for (int i = 0; i < 3; ++i) {
            mn = GetMin3(tri->vertices[0][i], tri->vertices[1][i], tri->vertices[2][i]);
            mx = GetMax3(tri->vertices[0][i], tri->vertices[1][i], tri->vertices[2][i]);
			if (mn < minPos[i]) minPos[i] = mn;
			if (mx > maxPos[i]) maxPos[i] = mx;
		}
	}
	bool InBox(Vector3f ori){
        double EPS = 1e-7;
		for (int i = 0; i < 3; i++)
			if ((ori[i] <= minPos[i] - EPS) || (ori[i] >= maxPos[i] + EPS))
				return false;
		return true;
	}
	double GetArea(){
		double a = maxPos[0] - minPos[0];
		double b = maxPos[1] - minPos[1];
		double c = maxPos[2] - minPos[2];
		return 2 * (a * b + b * c + c * a);
	}
    double intersect(const Ray &r) {
		double minDist = -1;
		Vector3f ray_O = r.getOrigin(), ray_V = r.getDirection();
		for (int i = 0; i < 3; i++) {
			double tmpT = -1;
			if (ray_V[i] >= 1e-7)
				tmpT = (minPos[i] - ray_O[i]) / ray_V[i];
			else if (ray_V[i] <= -1e-7)
				tmpT = (maxPos[i] - ray_O[i]) / ray_V[i];
			if (tmpT >= 1e-7) {
				Vector3f C = ray_O + ray_V * tmpT;
				if (InBox(C)) {
					double dist = (C - ray_O).length();
					if (minDist <= -1e-7 || minDist > dist)
						minDist = dist;
				}
			}
		}
		return minDist;
	}
};

class TriTreeNode {
public:
	Triangle** triangleList;
	int size, plane;
	double split;
	BoundBox box;
	TriTreeNode* ls;
	TriTreeNode* rs;

	TriTreeNode(){
		size = 0;
		plane = -1;
		split = 0;
		ls = rs = NULL;
	}
	~TriTreeNode(){
		for (int i = 0; i < size; i++)
			delete triangleList[i];
		delete triangleList;
		delete ls;
		delete rs;
	}
};

// Better to use pointers
class TriangleTree {
	void deleteTree(TriTreeNode* node){
		if (node->ls != NULL)
			deleteTree(node->ls);
		if (node->rs != NULL)
			deleteTree(node->rs);
		delete node;
	}
	void sortList(Triangle** triangleList, int l, int r, int i, bool minCoord);
	void build(TriTreeNode* node);
	bool searchTree(TriTreeNode* node, const Ray &r, Hit &h, double tmin);

public:
	TriTreeNode* root;
	TriangleTree(){
		root = new TriTreeNode;
	}
	~TriangleTree(){
		deleteTree(root);
	}
	void buildTree(){
		build(root);
		printf("Bound Box:\n");
		root->box.maxPos.print();
    	root->box.minPos.print();
	}
	bool intersect(const Ray &r, Hit &h, double tmin){
		return searchTree(root, r, h, tmin);
	}
};

class Mesh : public Object3D {
public:
    Mesh(const char *filename, Material *m, double scale);

	TriangleTree* tree;
	double scale=0.3;
    bool intersect(const Ray &r, Hit &h, double tmin) override;
	void getSize(std::string file);
	void getMtlSize(std::string file);
	void getMtl(std::string file);

private:
	int vSize, vtSize, vnSize, fSize, matSize;
	Vector3f* v;
	std::pair<double, double>* vt;
	Vector3f* vn;
	Triangle** triangleList;
	Material** mat;
	std::map<std::string, int> matMap;	
    
    // Normal can be used for light estimation
    void computeNormal();
};

#endif
