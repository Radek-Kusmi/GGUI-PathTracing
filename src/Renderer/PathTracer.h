#ifndef PATH_TRACER_H
#define PATH_TRACER_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../Core/Ray.h"
#include "../Scene/Mesh.h"
#include "../Utils/TextureLoader.h"

class PathTracer {
public:
    int width;
    int height;

    PathTracer(int renderWidth, int renderHeight);
    ~PathTracer();

    // Renderuje pojedynczą klatkę na CPU (na razie bez odbić - czysty kolor trafienia)
    void renderPass(
        const glm::vec3& camPos,
        const glm::vec3& camTarget,
        float fovDegrees,
        const Mesh& lighthouseMesh,
        const glm::mat4& lighthouseModel,
        const Mesh& iceMesh,
        const glm::mat4& iceModel,
        const CpuTexture& iceTexture
    );

    // Wyświetla bufor z RAM na ekranie za pomocą OpenGL
    void display();

private:
    std::vector<unsigned char> pixelBuffer; // RGBA8 w pamięci RAM
    GLuint screenTexture = 0;
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    GLuint quadShader = 0;

    glm::vec3 traceRay(
        const Ray& ray,
        const Mesh& lighthouseMesh,
        const glm::mat4& invLighthouseModel,
        const glm::mat3& lighthouseNormalMat,
        const Mesh& iceMesh,
        const glm::mat4& invIceModel,
        const glm::mat3& iceNormalMat,
        const CpuTexture& iceTexture
    );

    void initDisplayResources();
};

#endif // PATH_TRACER_H