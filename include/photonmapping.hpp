#ifndef PHOTONMAPPING_H
#define PHOTONMAPPING_H

#include "scene_parser.hpp"
#include "photon.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>
#include <algorithm>
#include <queue>
#include <map>

#define ran() ( double( rand() % RAND_MAX ) / RAND_MAX )
#define MAX_TRACING_DEPTH 8 
#define EPS 1e-7
#define HITPOINTOUTER 0.1


struct PhotonBeenFound {
	Vector3f position;
	int maxToFound;
    int foundNum;

	bool heapDone;
	double lim;
	Photon** photons;
    
    std::priority_queue<std::pair<double, Photon*>>* hp;

	PhotonBeenFound(Vector3f pos, int maxtf, double l){
        position = pos, maxToFound = maxtf, lim = l;
        foundNum = 0;
        heapDone = false;
        photons = new Photon*[maxtf + 1];
    }
};

Vector3f rotation(const Vector3f &target, const Vector3f &axis, double theta ) {
	double resx, resy, resz;
    double targetx = target.x(), targety = target.y(), targetz = target.z();
    double axisx = axis.x(), axisy = axis.y(), axisz = axis.z(); 
	double cost = cos( theta );
	double sint = sin( theta );
	resx += targetx * ( axisx * axisx + ( 1 - axisx * axisx ) * cost );
	resx += targety * ( axisx * axisy * ( 1 - cost ) - axisz * sint );
	resx += targetz * ( axisx * axisz * ( 1 - cost ) + axisy * sint );
	resy += targetx * ( axisy * axisx * ( 1 - cost ) + axisz * sint );
	resy += targety * ( axisy * axisy + ( 1 - axisy * axisy ) * cost );
	resy += targetz * ( axisy * axisz * ( 1 - cost ) - axisx * sint );
	resz += targetx * ( axisz * axisx * ( 1 - cost ) - axisy * sint );
	resz += targety * ( axisz * axisy * ( 1 - cost ) + axisx * sint );
	resz += targetz * ( axisz * axisz + ( 1 - axisz * axisz ) * cost );
	return Vector3f(resx, resy, resz);
}

// Construct KD-Tree
int nowd;
inline bool cmp(Photon a, Photon b) {
    return a.position[nowd] < b.position[nowd];
}

class PhotonMap {
public:
    PhotonMap(const int &emitPhoton, const int &maxInMap, const int &sample_photons, const int &sample_dist) {
        this->emitPhoton = emitPhoton;
        this->maxInMap = maxInMap;
        this->stored_photons = 0;
        this->sample_dist = sample_dist;
        this->sample_photons = sample_photons;
        this->photons = new Photon[maxInMap+1];
        // For KD-Tree
        box_min = Vector3f(inf, inf, inf);
        box_max = Vector3f(-inf, -inf, -inf);
    }
    void findPhoton(PhotonBeenFound* np, int p) {
        Photon *curphoton = &photons[p];
        nowd = curphoton->d;
        double dist = np->position[nowd] - curphoton->position[nowd];
        if (dist >= 0) {
            if(curphoton->rs) findPhoton(np, curphoton->rs);
            if (dist * dist < np->lim && curphoton->ls) 
                findPhoton(np, curphoton->ls);
        }
        else {
            if(curphoton->ls) findPhoton(np, curphoton->ls);
            if (dist * dist < np->lim && curphoton->rs) 
                findPhoton(np, curphoton->rs);
        } 

        double squareDis = (curphoton->position - np->position).squaredLength();
        if (squareDis > np->lim) return;

        if (np->foundNum < np->maxToFound)
            np->photons[++(np->foundNum)] = curphoton;
        else {
            if ( np->heapDone == false ) {
                np->hp = new std::priority_queue<std::pair<double, Photon*>>;
                for(int i = 1; i <= np->foundNum; ++i) 
                    np->hp->push(std::make_pair(-(np->photons[i]->position - np->position).squaredLength(), np->photons[i]));
                np->heapDone = true;
            }
            np->hp->push(std::make_pair(-squareDis, curphoton));
            np->hp->pop();
        }
    }
    void build(int &p, int l, int r) {
        nowd = 2;
        if (box_max.x() - box_min.x() >= box_max.y() - box_min.y() && box_max.x() - box_min.x() >= box_max.z() - box_min.z()) nowd = 0;
        else if (box_max.y() - box_min.y() >= box_max.z() - box_min.z()) nowd = 1;

        int mid = (l+r)>>1;
        std::nth_element(photons+l, photons+mid, photons+r+1, cmp);
        p=mid; photons[p].d = nowd;
        if(l < mid) {
            double rec = box_max[nowd];
            box_max[nowd] = photons[p].position[nowd];
            build(photons[p].ls, l, mid-1);
            box_max[photons[p].d] = rec;
        }
        if(r > mid) {
            double rec = box_min[nowd];
            box_min[nowd] = photons[p].position[nowd];
            build(photons[p].rs, mid+1, r);
            box_min[photons[p].d] = rec;
        }
    }
    void buildKDTree() {
        build(rt, 1, stored_photons);
    }
    void addPhoton(Photon photon) {
        if(stored_photons+1 > maxInMap) return;
        //printf("Done!");
        //printf("%d %d Done!\n", stored_photons, maxInMap);
        photons[++stored_photons] = photon;
        box_min = Vector3f(std::min( box_min.x(), photon.position.x()), std::min( box_min.y(), photon.position.y()),
                            std::min( box_min.z(), photon.position.z()));
        box_max = Vector3f(std::max( box_max.x(), photon.position.x()), std::max( box_max.y(), photon.position.y()),
                            std::max( box_max.z(), photon.position.z()));
    }
    Vector3f getIrradiance(Vector3f hitPoint, Vector3f hitNorm, double lim, int toFound) {
        // return Vector3f(0);
        Vector3f res(0);
        PhotonBeenFound np(hitPoint, toFound, lim*lim);

        findPhoton(&np, rt);
        if ( np.foundNum <= 8 ) return Vector3f(0); // threshold 8
        // printf("%d %d\n", np.foundNum, np.maxToFound);
        if (np.heapDone) {
            // printf("!\n");
            while(!np.hp->empty()) {
                auto q = np.hp->top(); np.hp->pop(); //printf("%.2lf ", -q.first);
                if ( Vector3f::dot(hitNorm, q.second->direction) < 0 )
                    res += q.second->power;
            }
            delete np.hp;
        }
        else
            for (int i = 1; i <= np.foundNum; i++ )
                if ( Vector3f::dot(hitNorm, np.photons[i]->direction) < 0 ) res += np.photons[i]->power;

        res *=  4 / (emitPhoton * np.lim);
        delete[] np.photons;
        return res;
    }
    ~PhotonMap() {
        delete[] photons;
    }

    int rt; // root of KD-Tree
    int emitPhoton;
    int maxInMap;
    int stored_photons;
    int sample_dist;
    int sample_photons;
    Photon* photons;
    Vector3f box_max;
    Vector3f box_min;
};

class PhotonMapping {
public:
    SceneParser* sceneparser;
    Group* baseGroup;
    PhotonMap* map;

    PhotonMapping(SceneParser* sceneparser) {
        this->sceneparser = sceneparser;
        this->baseGroup = sceneparser->getGroup();
    }

    // -------------------Forward---------------------

    void forwardDiffusion(Hit *hit, Photon photon){
        Material* material = hit->getMaterial();
        Vector3f hitNormed = hit->getNormal().normalized(), 
                 normVer = Vector3f::cross(hitNormed, Vector3f(1.1,0.2,0.23)).normalized();
        double theta = acos( sqrt( ran() ) ), phi = ran() * 2 * M_PI; 
        
        photon.direction = rotation(rotation(hitNormed, normVer, theta), hitNormed, phi).normalized();
        photon.position += HITPOINTOUTER * photon.direction;
        photon.power = photon.power * material->mColor / material->getColorPower();
    }
    void forwardReflection(Hit *hit , Photon photon){
        Material* material = hit->getMaterial();
        Vector3f hitNormed = hit->getNormal().normalized();

        photon.direction = (2*Vector3f::dot(hitNormed, photon.direction.normalized())*hitNormed + photon.direction.normalized()).normalized();
        photon.position += HITPOINTOUTER*photon.direction;
        photon.power = photon.power * material->mColor / hit->getMaterial()->getColorPower();
    }
	void forwardRefraction(Hit *hit , Photon photon){
        Material* material = hit->getMaterial();
        Vector3f hitNormed = hit->getNormal().normalized(), nRayed = -photon.direction.normalized();
        double tmpN;
        if (photon.currentN <= 1+EPS)
            tmpN = 1/material->refractionN;
        else
            tmpN = material->refractionN;

        double nnt = tmpN, ddn = Vector3f::dot(-nRayed, hitNormed), cos2t=1-nnt*nnt*(1-ddn*ddn);

        if (cos2t < EPS) {
            Vector3f reflDir = (2*Vector3f::dot(hitNormed, nRayed)*hitNormed - nRayed).normalized();
            photon.position += HITPOINTOUTER*hitNormed;
            photon.direction = reflDir;
            photon.position += HITPOINTOUTER*photon.direction;
        }
        else {
            double cosI = -Vector3f::dot(hitNormed, photon.direction);
	        Vector3f refrDir =  photon.direction * nnt + hitNormed * ( nnt * cosI - sqrt( cos2t ) );
            refrDir = refrDir.normalized();
            if (photon.currentN <= 1+EPS) photon.currentN = material->refractionN;
            else    photon.currentN = 1;
            photon.absorb = material->absorption;
            photon.direction = refrDir;
            photon.position += HITPOINTOUTER*photon.direction;
        }
    }
    void forwardTracing(Photon photon, int depth) {
        for(int depth = 1; depth <= MAX_TRACING_DEPTH; ++depth) {
            
            Hit hit;
            Ray R = Ray(photon.position, photon.direction);

            bool isIntersect = baseGroup->intersect(R, hit, 0);
            if (isIntersect) {
                
                Vector3f hitNormed = hit.getNormal().normalized(),
                        hitPoint = R.pointAtParameter(hit.getT());
                photon.position = hitPoint;
                
                // Diffusion -> store the photon
                Material* material = hit.getMaterial();
                if (material->diffusion > EPS)
                    map->addPhoton(photon);
                // Russian Roulette
                double tmp = ran();
                double P_diff = material->diffusion * material->getColorPower();
                double P_refl = material->reflection * material->getColorPower();
                
                if (tmp < P_diff) forwardDiffusion(&hit, photon);
                else if(tmp < P_diff + P_refl) forwardReflection(&hit, photon);
                else {   
                    double P_refr = material->refraction;
                    if ( photon.currentN != 1 ) {
                        Vector3f absor = photon.absorb*(-hit.getT()*photon.direction.length());
                        Vector3f trans = Vector3f(exp( absor.x() ), exp( absor.y()), exp( absor.z()));
                        double tPower = (trans.x()+trans.y()+trans.z())/3;
                        P_refr *= tPower;
                        photon.power = photon.power * trans / tPower;
                    }
                    if (tmp < P_diff + P_refl + P_refr) forwardRefraction(&hit, photon);
                    else break;
                }
            }
            else break;
        }
    }

    // -------------------Backward--------------------

    Vector3f backwardDiff( Hit *hit, Ray *r) {
        Vector3f color(0);
        if (hit->getMaterial()->texture == NULL)
            color = hit->getMaterial()->mColor;
        else
            color = hit->getMaterial()->getTextureColor(hit->getX(), hit->getY());
        Vector3f res = color * sceneparser->getBackgroundColor() * hit->getMaterial()->diffusion;
        res += color * map->getIrradiance(r->pointAtParameter(hit->getT()), hit->getNormal().normalized(), map->sample_dist, map->sample_photons ) * hit->getMaterial()->diffusion;
        return res;
    }
    
    Vector3f backwardRefl( Hit *hit, Ray *r, int depth) {
        Vector3f hitNormed = hit->getNormal().normalized(), nRayed = -r->getDirection().normalized();
        Vector3f reflDir = (2*Vector3f::dot(hitNormed, nRayed)*hitNormed - nRayed).normalized();
        return backwardTracing(Ray(r->pointAtParameter(hit->getT())+HITPOINTOUTER*reflDir, reflDir), depth + 1) * hit->getMaterial()->mColor * hit->getMaterial()->reflection;
    }

    Vector3f backwardRefr( Hit *hit, Ray *r, int depth, double cN = 1, Vector3f cAb = Vector3f(0)) {
        Vector3f hitPoint = r->pointAtParameter(hit->getT());
        Vector3f hitNormed = hit->getNormal().normalized(), nRayed = -r->getDirection().normalized();
        double tmpN;
        if (cN <= 1+EPS)
            tmpN = 1/hit->getMaterial()->refractionN;
        else
            tmpN = hit->getMaterial()->refractionN;

        Vector3f dir(0), newAb=cAb;
        double newN = cN;
        double nnt = tmpN, ddn = Vector3f::dot(-nRayed, hitNormed), cos2t=1-nnt*nnt*(1-ddn*ddn);

        if (cos2t < EPS){
            Vector3f reflDir = (2*Vector3f::dot(hitNormed, nRayed)*hitNormed - nRayed).normalized();
            dir = reflDir;
            hitPoint += HITPOINTOUTER*reflDir;
        }

        else{
            double cosI = -Vector3f::dot(hitNormed, -nRayed);
	        Vector3f refrDir =  -nRayed * nnt + hitNormed * ( nnt * cosI - sqrt( cos2t ) );
            refrDir = refrDir.normalized();
            if (newN <= 1 + EPS)  newN = hit->getMaterial()->refractionN;
            else newN = 1;
            newAb = hit->getMaterial()->absorption;
            dir = refrDir;
            hitPoint += HITPOINTOUTER*refrDir;
        }
        
        Vector3f rcol = backwardTracing( Ray(hitPoint, dir), depth + 1, newN, newAb);
        if ( cN <= 1+EPS )
            return rcol * hit->getMaterial()->refraction;
        Vector3f absor = cAb*(hit->getT()*-r->getDirection().length());
        Vector3f trans = Vector3f( exp( absor.x() ) , exp( absor.y()) , exp( absor.z()));
        return rcol * trans * hit->getMaterial()->refraction;
    }

    Vector3f backwardTracing(Ray R, int depth, double currentN = 1, Vector3f cAbsorb = Vector3f(0)){
        if (depth > MAX_TRACING_DEPTH) return Vector3f(0);
        Hit hit;
        bool isIntersect = baseGroup->intersect(R, hit, 0);
        double tmpT = 1e6;
        int lIdx = -1;
        for (int li = 0 ; li < sceneparser->getNumLights(); ++li) {
            int temp = sceneparser->getLight(li)->isHit(R.getOrigin(),R.getDirection());
            if (temp != 0 && temp < tmpT)
                lIdx = li, tmpT = temp;
        }
        
        Vector3f res;
        if (depth == 1) res = sceneparser->getBackgroundColor();
        else res = Vector3f(0);
        if ((lIdx != -1) && (!isIntersect || hit.getT() > tmpT))
            res += Vector3f(1);
        if ( isIntersect ) {
            // printf("isIntersect\n");
            if ( hit.getMaterial()->diffusion > EPS ) res += backwardDiff( &hit, &R);
            if ( hit.getMaterial()->reflection > EPS ) res += backwardRefl( &hit, &R, depth);
            if ( hit.getMaterial()->refraction > EPS ) res += backwardRefr( &hit, &R, depth, currentN, cAbsorb);
        }
        if ( depth == 1 ) res = Vector3f( std::min( res[0] , double(1.0) ) , std::min( res[1] , double(1.0)) , std::min( res[2] , double(1.0) ) );
        return res;        
    }
};

#endif //PHOTONMAPPING_H
