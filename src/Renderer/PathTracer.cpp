#include "PathTracer.h"
#include "../Core/RayHit.h"
#include <cmath>
#include <iostream>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <omp.h>
#include <GLFW/glfw3.h>

PathTracer::PathTracer(int renderWidth, int renderHeight)
    : width(renderWidth), height(renderHeight) {
    pixelBuffer.resize(width * height * 4, 0);
    initDisplayResources();
}

PathTracer::~PathTracer() {
    if (screenTexture) glDeleteTextures(1, &screenTexture);
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (quadShader) glDeleteProgram(quadShader);
}

glm::vec3 PathTracer::traceRay(
    const Ray& ray,
    const Mesh& lighthouseMesh,
    const glm::mat4& invLighthouseModel,
    const glm::mat3& lighthouseNormalMat,
    const Mesh& iceMesh,
    const glm::mat4& invIceModel,
    const glm::mat3& iceNormalMat,
    const CpuTexture& iceTexture
) {
    RayHit closestHit;
    bool hitAnything = false;
    float maxDist = 1000.0f;

    // 1. Sprawdzamy kolizję z latarnią (używając gotowych invModel i normalMat)
    RayHit tempHit;
    if (intersectMesh(ray, lighthouseMesh, invLighthouseModel, lighthouseNormalMat, 0.001f, maxDist, tempHit, false)) {
        hitAnything = true;
        maxDist = tempHit.t;
        closestHit = tempHit;
    }

    // 2. Sprawdzamy kolizję z taflą lodu (szukamy obiektu bliżej kamery niż obecny maxDist)
    if (intersectMesh(ray, iceMesh, invIceModel, iceNormalMat, 0.001f, maxDist, tempHit, true)) {
        hitAnything = true;
        maxDist = tempHit.t;
        closestHit = tempHit;
    }

    // 3. Jeśli promień w coś trafił -> zwracamy czysty kolor obiektu (Albedo Pass)
    if (hitAnything) {
        if (closestHit.isIce) {
            return iceTexture.sample(closestHit.texCoords.x, closestHit.texCoords.y);
        }
        return closestHit.color;
    }

    // 4. Jeśli promień w nic nie trafił -> tło nieba (zgodne z OpenGL)
    return glm::vec3(0.08f, 0.11f, 0.18f);
}

void PathTracer::renderPass(
    const glm::vec3& camPos,
    const glm::vec3& camTarget,
    float fovDegrees,
    const Mesh& lighthouseMesh,
    const glm::mat4& lighthouseModel,
    const Mesh& iceMesh,
    const glm::mat4& iceModel,
    const CpuTexture& iceTexture
) {
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    float theta = glm::radians(fovDegrees);
    float halfHeight = std::tan(theta * 0.5f);
    float halfWidth = aspectRatio * halfHeight;

    glm::vec3 w = glm::normalize(camPos - camTarget);
    glm::vec3 u = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), w));
    glm::vec3 v = glm::cross(w, u);

    glm::vec3 lowerLeftCorner = camPos - halfWidth * u - halfHeight * v - w;
    glm::vec3 horizontal = 2.0f * halfWidth * u;
    glm::vec3 vertical = 2.0f * halfHeight * v;

    // Prekalkulacja odwrotności macierzy modeli i macierzy normalnych RAZ na całą klatkę
    glm::mat4 invLighthouseModel = glm::inverse(lighthouseModel);
    glm::mat3 lighthouseNormalMat = glm::transpose(glm::mat3(invLighthouseModel));

    glm::mat4 invIceModel = glm::inverse(iceModel);
    glm::mat3 iceNormalMat = glm::transpose(glm::mat3(invIceModel));

    int numThreads = omp_get_max_threads();
    std::system("cls");
    std::cout << "\n[PathTracer] Renderowanie (" << width << "x" << height << ") na " << numThreads << " watkach CPU..." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();

    std::atomic<int> completedRows{ 0 };

    // Rozdzielenie wierszy obrazu na wszystkie rdzenie procesora
#pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float s = (x + 0.5f) / static_cast<float>(width);
            float t = (y + 0.5f) / static_cast<float>(height);

            glm::vec3 dir = lowerLeftCorner + s * horizontal + t * vertical - camPos;
            Ray ray(camPos, dir);

            glm::vec3 color = traceRay(
                ray,
                lighthouseMesh, invLighthouseModel, lighthouseNormalMat,
                iceMesh, invIceModel, iceNormalMat,
                iceTexture
            );

            int idx = (y * width + x) * 4;
            pixelBuffer[idx + 0] = static_cast<unsigned char>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
            pixelBuffer[idx + 1] = static_cast<unsigned char>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
            pixelBuffer[idx + 2] = static_cast<unsigned char>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
            pixelBuffer[idx + 3] = 255;
        }

        int done = ++completedRows;
        // Tylko główny wątek (ID == 0) aktualizuje konsolę i odświeża kolejkę zdarzeń okna
        if (omp_get_thread_num() == 0 && (done % 10 == 0 || done == height)) {
            int progress = static_cast<int>((static_cast<float>(done) / height) * 100.0f);
            std::cout << "\r[PathTracer] Postep: " << progress << "% (" << done << "/" << height << " linii)" << std::flush;
            glfwPollEvents();
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;
    std::cout << "\r[PathTracer] Postep: 100% (" << height << "/" << height << " linii) - Czas: " << elapsed.count() << " s.      " << std::endl;

    // Aktualizacja tekstury OpenGL danymi wyliczonymi przez CPU
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer.data());
}

void PathTracer::display() {
    glDisable(GL_DEPTH_TEST);
    glUseProgram(quadShader);
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void PathTracer::initDisplayResources() {
    // 1. Tekstura ekranowa
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2. Pełnoekranowy prostokąt (2 trójkąty)
    float quadVertices[] = {
        // pos        // uv
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    // 3. Prosty wbudowany shader 2D
    const char* vsSrc = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoords;
        out vec2 TexCoords;
        void main() {
            TexCoords = aTexCoords;
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
        }
    )";

    const char* fsSrc = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoords;
        uniform sampler2D screenTex;
        void main() {
            FragColor = texture(screenTex, TexCoords);
        }
    )";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, nullptr);
    glCompileShader(fs);

    quadShader = glCreateProgram();
    glAttachShader(quadShader, vs);
    glAttachShader(quadShader, fs);
    glLinkProgram(quadShader);

    glDeleteShader(vs);
    glDeleteShader(fs);
}