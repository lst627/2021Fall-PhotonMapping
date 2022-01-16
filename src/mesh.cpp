#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>

#define EPS 1e-7

void Mesh::getMtlSize(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;

	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;
		if (var == "newmtl")
			matSize++;
	}
	fin.close();
	mat = new Material*[matSize + 1];
	mat[0] = new Material;
}

void Mesh::getSize(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;
	
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;
		if (var == "mtllib") {
			std::string mtlFile;
			fin2 >> mtlFile;
			getMtlSize(mtlFile);
		}
		if (var == "v")
			vSize++;
		if (var == "vt")
			vtSize++;
		if (var == "vn")
			vnSize++;
		if (var == "f") {
			int vertexCnt = 0;
			std::string var;
			while (fin2 >> var)
				vertexCnt++;
			fSize += std::max(0, vertexCnt - 2);
		}
	}
	fin.close();
	v = new Vector3f[vSize + 1];
	vt = new std::pair<double, double>[vtSize + 1];
	if (vnSize == 0)
		vn = new Vector3f[vSize + 1];
	else
		vn = new Vector3f[vnSize + 1];
	triangleList = new Triangle*[fSize];
}

void Mesh::getMtl(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;
	int matCnt = 0;
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;

		if (var == "newmtl") {
			std::string matName;
			fin2 >> matName;
			matMap[matName] = ++matCnt;
			mat[matCnt] = new Material();
		}
		if (var == "Kd") {
            Vector3f color(0);
            fin2 >> color[0] >> color[1] >> color[2];
			mat[matCnt]->mColor = color;
			mat[matCnt]->diffusion = std::max(mat[matCnt]->mColor[0], std::max(mat[matCnt]->mColor[1], mat[matCnt]->mColor[2]));
			mat[matCnt]->mColor = mat[matCnt]->mColor/mat[matCnt]->diffusion;
		}
		if (var == "Ks") {
			fin2 >> mat[matCnt]->reflection;
		}
		if (var == "Tf") {
            Vector3f absorb(0);
            fin2 >> absorb[0] >> absorb[1] >> absorb[2];
			mat[matCnt]->absorption = absorb;
			if ((mat[matCnt]->absorption[0]+mat[matCnt]->absorption[1]+mat[matCnt]->absorption[2])/3.0 < 1 - EPS) {
				mat[matCnt]->refraction = 1;
			}
		}
		if (var == "Ni") {
			fin2 >> mat[matCnt]->refractionN;
		}
	}
	fin.close();
}

Mesh::Mesh(const char *filename, Material *material, double scale) : Object3D(material) {
	this->scale = scale;
    vSize = vtSize = vnSize = fSize = matSize = 0;
    tree = new TriangleTree;
    std::string file = std::string(filename);
    getSize(file);
	std::ifstream fin(file.c_str());
	std::string order;

	int matID = -1;
	int vCnt = 0, vtCnt = 0, vnCnt = 0, fCnt = 0;
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;

		if (var == "mtllib") {
			std::string mtlFile;
			fin2 >> mtlFile;
			getMtl(mtlFile);
		}

		if (var == "usemtl") {
			std::string matName;
			fin2 >> matName;
			matID = matMap[matName];
		}
		if (var == "v") {
			vCnt++;
			v[vCnt].Input(fin2);
		}
		if (var == "vt") {
			vtCnt++;
			fin2 >> vt[vtCnt].second >> vt[vtCnt].first;
		}
		if (var == "vn") {
			vnCnt++;
			vn[vnCnt].Input(fin2);
		}
		if (var == "f") {
			Triangle* tri = triangleList[fCnt] = new Triangle(NULL);
			if (matID != -1)
				tri->material = mat[matID];
			else
				tri->material = this->material;
			std::string str;
			for (int i = 0; fin2 >> str; ++i) {
				int bufferLen = 0, buffer[3];
				buffer[0] = buffer[1] = buffer[2] = -1;
				for (int s = 0, t = 0; t < (int)str.length(); ++t)
					if (t + 1 >= (int)str.length() || str[t + 1] == '/') {
						buffer[bufferLen++] = atoi(str.substr(s, t - s + 1).c_str());
						s = t + 2;
					}
				int j = i;
				if (i >= 3) {
					j = 2;
					tri = triangleList[fCnt] = new Triangle(NULL);
					*tri = *triangleList[fCnt - 1];
					tri->vertices[1] = tri->vertices[2];
					tri->textureVertex[1] = tri->textureVertex[2];
					tri->normalVectorID[1] = tri->normalVectorID[2];
				}
				if (buffer[0] > 0) {
					Vector3f p = v[buffer[0]];
                    p *= 1/this->scale;
                    if (j == 0) tri->vertices[0] = p;
                    else if (j == 1) tri->vertices[1] = p;
                    else if (j == 2) tri->vertices[2] = p;
				}
				if (buffer[1] > 0)
					tri->textureVertex[j] = buffer[1];
				if (buffer[2] > 0)
					tri->normalVectorID[j] = buffer[2];
				if (i >= 2)
					tri->setpar(), fCnt++;
			}
		}
	}
	fin.close();

	TriTreeNode* root = tree->root;
	root->size = fCnt;
	root->triangleList = new Triangle*[root->size];
	// for(int i = 0; i < fCnt; ++ i)
	// 	triangleList[i]->print();
	for (int i = 0; i < root->size; ++i) {
		root->triangleList[i] = triangleList[i];
		root->box.UpdateBox(triangleList[i]);
	}
	tree->buildTree();
    printf("vCnt: %d, vtCnt: %d, vnCnt: %d, fCnt: %d, rootSize: %d\n", vCnt, vtCnt, vnCnt, fCnt, root->size);
}

void TriangleTree::sortList(Triangle** triangleList, int left, int right, int idx, bool isMin) {
	double (Triangle::*GetCoord)(int) = isMin ? &Triangle::MinCoord : &Triangle::MaxCoord;
	if (left >= right) return;
	Triangle* key = triangleList[(left + right) >> 1];
	int i,j;
	for (i = left, j = right; i <= j;) {
		for(; j >= left && (key->*GetCoord)(idx) < (triangleList[j]->*GetCoord)(idx); --j);
		for(; i <= right && (triangleList[i]->*GetCoord)(idx) < (key->*GetCoord)(idx); ++i);
		if (i <= j) {
			std::swap(triangleList[i], triangleList[j]);
			i++, j--;
		}
	}
	if(i < right) sortList(triangleList, i, right, idx, isMin);
	if(left < j) sortList(triangleList, left, j, idx, isMin);
}

void TriangleTree::build(TriTreeNode* node) {
	Triangle** minNode = new Triangle*[node->size];
	Triangle** maxNode = new Triangle*[node->size];
	for (int i = 0; i < node->size; ++i)
		minNode[i] = node->triangleList[i], maxNode[i] = node->triangleList[i];
	
	double thisCost = node->box.GetArea() * (node->size - 1);
	double mncost = thisCost;
	int bestc = -1, leftSize = 0, rightSize = 0;
	double bests = 0;
	for (int idx = 0; idx < 3; ++idx) {
		sortList(minNode, 0, node->size-1, idx, true);
		sortList(maxNode, 0, node->size-1, idx, false);
		BoundBox leftBox = node->box;
		BoundBox rightBox = node->box;

		for (int i = 0, j = 0; i < node->size; ++i) {
			double split = minNode[i]->MinCoord(idx);
			leftBox.maxPos[idx] = split;
			rightBox.minPos[idx] = split;
			for ( ; j < node->size && maxNode[j]->MaxCoord(idx) <= split + EPS; ++j);
			double cost = leftBox.GetArea() * i + rightBox.GetArea() * (node->size - j);
			if (cost < mncost) {
				mncost = cost, bestc = idx, bests = split;
				leftSize = i;
				rightSize = node->size - j;
			}
		}

		for (int i = 0, j = 0; i < node->size; ++i) {
			double split = maxNode[i]->MaxCoord(idx);
			leftBox.maxPos[idx] = split;
			rightBox.minPos[idx] = split;
			for ( ; j < node->size && minNode[j]->MinCoord(idx) <= split - EPS; ++j);
			double cost = leftBox.GetArea() * j + rightBox.GetArea() * (node->size - i);
			if (cost < mncost) {
				mncost = cost, bestc = idx, bests = split;
				leftSize = j;
				rightSize = node->size - i;
			}
		}
	}

	delete minNode;
	delete maxNode;
	
	if (bestc == -1) return;

	leftSize = rightSize = 0;
	for (int i = 0; i < node->size; ++i) {
		if (node->triangleList[i]->MinCoord(bestc) <= bests - EPS || node->triangleList[i]->MaxCoord(bestc) <= bests + EPS)
			leftSize++;
		if (node->triangleList[i]->MaxCoord(bestc) >= bests + EPS || node->triangleList[i]->MinCoord(bestc) >= bests - EPS)
			rightSize++;
	}
	BoundBox leftBox = node->box, rightBox = node->box;
	leftBox.maxPos[bestc] = rightBox.minPos[bestc] = bests;
	// *****
	double cost = leftBox.GetArea() * leftSize + rightBox.GetArea() * rightSize;

	if (cost < thisCost) {
		node->plane = bestc, node->split = bests;

		node->ls = new TriTreeNode;
		node->ls->box = node->box;
		node->ls->box.maxPos[node->plane] = node->split;
		
		node->rs = new TriTreeNode;
		node->rs->box = node->box;
		node->rs->box.minPos[node->plane] = node->split;
		
		node->ls->triangleList = new Triangle*[leftSize];
		node->rs->triangleList = new Triangle*[rightSize];
		int leftCnt = 0, rightCnt = 0;
		for (int i = 0; i < node->size; ++i) {
			if (node->triangleList[i]->MinCoord(node->plane) <= node->split - EPS || node->triangleList[i]->MaxCoord(node->plane) <= node->split + EPS)
				node->ls->triangleList[leftCnt++] = node->triangleList[i];
			if (node->triangleList[i]->MaxCoord(node->plane) >= node->split + EPS || node->triangleList[i]->MinCoord(node->plane) >= node->split - EPS)
				node->rs->triangleList[rightCnt++] = node->triangleList[i];
		}
		node->ls->size = leftSize;
		node->rs->size = rightSize;

		build(node->ls);
		build(node->rs);
	}
}

bool TriangleTree::searchTree(TriTreeNode* node, const Ray &r, Hit &h, double tmin){
    Vector3f ray_O = r.getOrigin(), ray_V = r.getDirection();
	if (!(node->box.InBox(ray_O)) && node->box.intersect(r) <= -EPS)
		return false;
    
	if (node->ls == NULL && node->rs == NULL) {
    	bool treeIntersect = false;
		for (int i = 0; i < node->size; ++i) {
            bool flag;
            Hit triHit = Hit();
			flag = node->triangleList[i]->intersect(r, triHit, tmin);
			// if(flag) node->triangleList[i]->print(); 
			if (flag) {
				if( node->box.InBox(r.pointAtParameter(triHit.getT())) && (!treeIntersect || triHit.getT() < h.getT()))
					h = triHit, treeIntersect = true;
            }
		}
		return treeIntersect;
	}
	
	if (node->ls->box.InBox(ray_O)) {
		if (searchTree(node->ls, r, h, tmin)) return true;
		return searchTree(node->rs, r, h, tmin);
	}
	if (node->rs->box.InBox(ray_O)) {
		if (searchTree(node->rs, r, h, tmin)) return true;
		return searchTree(node->ls, r, h, tmin);
	}

	double leftDist = node->ls->box.intersect(r);
	double rightDist = node->rs->box.intersect(r);
	if (rightDist <= -EPS)
		return searchTree(node->ls, r, h, tmin);
	if (leftDist <= -EPS)
		return searchTree(node->rs, r, h, tmin);
	
	if (leftDist < rightDist) {
		if (searchTree(node->ls, r, h, tmin)) return true;
		return searchTree(node->rs, r, h, tmin);
	}
    else {
		if (searchTree(node->rs, r, h, tmin)) return true;
    	return searchTree(node->ls, r, h, tmin);
	}
}

bool Mesh::intersect(const Ray &r, Hit &h, double tmin) {
    return tree->intersect(r, h, tmin);
}