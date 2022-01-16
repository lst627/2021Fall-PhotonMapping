#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include "image.hpp"

// TODO: Implement Shade function that computes Phong introduced in class.
class Material {
public:

    Material(){
        diffusion=0, shininess=0;
        reflection=0;
        refraction=0, refractionN=0;
        textcoff=0;
        mColor = Vector3f(0);
        absorption = Vector3f(0);
        textDirection = Vector3f(0);
        texture = NULL;
        tw=0, th=0;
    }

    explicit Material(const Vector3f &mColor,const Vector3f &textDir=Vector3f::ZERO, const Vector3f &absorption = Vector3f::ZERO, double diffusion = 1, double shininess = 0, double reflection = 0, double refraction = 0, double refractionN = 1, char* fName = NULL, double tc = 1) :
            mColor(mColor), absorption(absorption), diffusion(diffusion), shininess(shininess), reflection(reflection), refraction(refraction), refractionN(refractionN), textcoff(tc) {
        
        if (*fName != 0){
            texture = Image::LoadPPM(fName);
            //std::cout << fName << "***"<<texture->Width()<<" "<<texture->Height()<<"\n";
            this->tw = texture->Width();
            this->th = texture->Height();
            this->textDirection = textDir;
            //printf("%d %d\n", this->tw, this->th);
        }
        else {
            tw = 0;
            th = 0;
            texture = NULL;
        }
    }
    
    double getColorPower() const{
        return (mColor.x()+mColor.y()+mColor.z())/3;
    }

    Vector3f getTextureColor(double u, double v){
        int x = (int)(u*textcoff)%tw;
        if (x<0) x += tw;
        int y = (int)(v*textcoff)%th;
        if (y<0) y += th;
        Vector3f color = texture->GetPixel(x, y);
        // color.print();
        return color;
    }

    virtual ~Material() = default;
    
    double diffusion, shininess, reflection, refraction, refractionN, textcoff;
    Vector3f mColor, absorption, textDirection;
    Image *texture = NULL;
    int tw=0, th=0;

    // double clamp(double x) {
    //     if (x < 0) return 0;
    //     return x;
    // }

    // Vector3f Shade(const Ray &ray, const Hit &hit,
    //                const Vector3f &dirToLight, const Vector3f &lightColor) {
    //     Vector3f shaded = Vector3f::ZERO;
    //     Vector3f N = hit.getNormal().normalized();
    //     Vector3f V = -ray.getDirection().normalized();
    //     Vector3f L = dirToLight.normalized();
    //     Vector3f R = 2 * Vector3f::dot(N, L) * N - L;
    //     R = R.normalized();  // !!!!!!

    //     Vector3f sum = diffuseColor * clamp(Vector3f::dot(L,N)) + specularColor * pow((double)clamp(Vector3f::dot(V,R)),(double)shininess);
    //     shaded = lightColor * sum;

    //     return shaded;
    // }

};


#endif // MATERIAL_H
