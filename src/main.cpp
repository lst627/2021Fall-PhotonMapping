#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "hit.hpp"
#include "photonmapping.hpp"

#include <string>

using namespace std;

int main(int argc, char *argv[]) {
    //---------------------I/O--------------------------
    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3) {
        cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];  // only bmp is allowed.
    cout << "Hello! Computer Graphics!" << endl;

    //-------------------Parameters---------------------
    SceneParser sceneParser(argv[1]);
    int emitPhoton = 3000000;
    int maxInMap = 10000000;
    int sample_photons= 150000;
	int sample_dist= 1;	
    bool antialiasing = false;
    double aliasing_samples[9][2] = {{-0.5,-0.5}, {-0.5,0}, {-0.5, 0.5}, {0,-0.5}, {0,0}, {0, 0.5}, {0.5,-0.5}, {0.5,0}, {0.5, 0.5}};

    // -------------------Build Map---------------------
    printf("Start building!!!\n");

    PhotonMapping photonMapping(&sceneParser);
    photonMapping.map = new PhotonMap(emitPhoton, maxInMap, sample_photons, sample_dist);
    double power = 0;
    for (int li = 0; li < sceneParser.getNumLights(); ++li) {
        Light* light = sceneParser.getLight(li);
        power += light->getColorPower();
    }
    double photon_power = power / emitPhoton;
    int emited_photons = 0;
    for (int li = 0 ; li < sceneParser.getNumLights(); ++li) {
        Light* light = sceneParser.getLight(li);
        // # photons is in proportional to light power
        long iter = long(light->getColorPower()/photon_power);

        #pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i <= iter; i ++){
            // printf("%d\n", i);
            emited_photons += 1;
            Photon photon = sceneParser.getLight(li)->EmitPhoton();
            photon.power *= power;
            photonMapping.forwardTracing(photon, 1);
        }
    }
    photonMapping.map->buildKDTree();

    printf("Build Finished!\n");
    // -------------------Rendering---------------------
    Camera *camera = sceneParser.getCamera();
    Vector3f gcolor = sceneParser.getBackgroundColor();
    int W = camera->getWidth(), H = camera->getHeight();
    Image renderedImg(W, H);

    Vector3f finalColor(0);
    #pragma omp parallel for schedule(dynamic, 1) private(finalColor)
    for (int x = 0; x < W; ++x) {
        printf("%d\n", x);
        for (int y = 0; y < H; ++y) {
            if(((x+1)%80) == 0 && ((y+1)%100) == 0)
                printf("%d %d\n", x, y);
            // printf("%d %d\n", x, y);
            Ray camRay = camera->generateRay(Vector2f(x, y));

            if (!camera->isDOF){
                finalColor = photonMapping.backwardTracing(camRay, 1);
                renderedImg.SetPixel(x, y, finalColor);
            }
            else{
                finalColor = Vector3f(0);
                Vector3f camCenter = camera->center;
                Vector3f focusPoint = camRay.pointAtParameter(camera->focusDist);
                Vector3f dirX = Vector3f::cross(camera->direction, camera->up).normalized();
                Vector3f dirY = Vector3f::cross(camera->direction, dirX).normalized();
                for (int i = 0; i < camera->lenSampleNum; ++i) {
                    // modify the ray for DOF
                    double dx=( ran() * 2 - 1 ), dy = ( ran() * 2 - 1 ), square = dx*dx+dy*dy;
                    dx /= sqrt(square), dy /= sqrt(square);
                    Vector3f newO = camCenter + camera->lenRadius*dx*dirX + camera->lenRadius*dy*dirY; // lenRadius: lens aperture
                    Ray newRay = Ray(newO, (focusPoint-newO).normalized());
                    finalColor += photonMapping.backwardTracing(newRay, 1);
                }
                renderedImg.SetPixel(x, y, finalColor/camera->lenSampleNum);
            }
        }
    }
    // Post-Processing: Anti-Aliasing
    int Cnt = 0;
    if (antialiasing) {
        double thres = 0.3;
        double** G = new double*[W+2];
        for(int i = 0; i < W+2; ++i)
            G[i] = new double[H+2];
        for(int x = 1; x < W+1; ++x) 
            for(int y = 1; y < H+1; ++y)
                G[x][y] = renderedImg.GetPixel(x-1, y-1).avg();

        #pragma omp parallel for schedule(dynamic, 1) private(finalColor)
        for(int x = 1; x < W+1; ++x) {
            printf("%d\n", x);
            for(int y = 1; y < H+1; ++y) {
                double Gx = G[x-1][y+1]+2*G[x][y+1]+G[x+1][y+1]-G[x-1][y-1]-2*G[x][y-1]-G[x+1][y-1];
                double Gy = G[x-1][y-1]+2*G[x-1][y]+G[x-1][y+1]-G[x+1][y-1]-2*G[x+1][y]-G[x+1][y+1];
                if(Gx*Gx + Gy*Gy > thres) {
                    // Anti-aliasing without DOF !!!!!
                    Cnt ++;
                    finalColor = Vector3f(0);
                    for(int k = 0; k < 9; ++k) {
                        Ray camRay = camera->generateRay(Vector2f(x-1+aliasing_samples[k][0], y-1+aliasing_samples[k][1]));
                        if (!camera->isDOF)
                            finalColor += photonMapping.backwardTracing(camRay, 1);
                    }
                    finalColor = finalColor / 9.0;
                    renderedImg.SetPixel(x-1, y-1, finalColor);
                }
            }
        }
        for(int i = 0; i < W+2; ++i)
            delete[] G[i];
        delete[] G;
    }
    
    printf("%d\n", Cnt);
    renderedImg.SaveImage(argv[2]);
    return 0;
}

