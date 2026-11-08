#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC

#include <iostream>
#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Scene/Scene.h"
#include "Scene/IcePlane.h"
#include "Utils/ShaderLoader.h"
#include "Utils/TextureLoader.h"

int main() {
    if (!glfwInit()) {
        std::cerr << "Błąd inicjalizacji GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(2880, 1620, "Scena 3D - Latarnia i Lód", NULL, NULL);
    if (!window) {
        std::cerr << "Błąd tworzenia okna GLFW!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Błąd inicjalizacji GLEW!" << std::endl;
        return -1;
    }

    // 1. Inicjalizacja Sceny C++ (RAM pod Path Tracer i OpenGL)
    Scene scene;

    // Dodanie Latarni z pliku OBJ
    if (!scene.addMesh("assets/models/Lighthouse1/lightHouse.obj")) {
        std::cerr << "Nie udało się wczytać modelu latarni." << std::endl;
        return -1;
    }

    // Dodanie proceduralnego wyboistego jeziora lodu
    // (szerokość: 80, głębokość: 80, rozdzielczość: 100x100, wysokość fal: 0.35f)
    Mesh icePlane = generateIcePlane(80.0f, 80.0f, 100, 0.35f);
    scene.addMesh(icePlane);

    // Dodanie źródła światła z latarni
    scene.addLight(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f, 0.9f, 0.7f), 10.0f);

    // Drukujemy podsumowanie wbudowanych trójkątów na CPU
    scene.printInfo();

    // Przygotowanie buforów VBO/VAO w OpenGL dla wczytanych obiektów pod podgląd
    for (auto& mesh : scene.meshes) {
        mesh.setupMesh();
    }

    // 2. Wczytanie i kompilacja shaderów
    std::string vertCode = loadShaderSource("assets/shaders/default.vert");
    std::string fragCode = loadShaderSource("assets/shaders/default.frag");
    const char* vertexShaderSource = vertCode.c_str();
    const char* fragmentShaderSource = fragCode.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glEnable(GL_DEPTH_TEST);

    // Zmienne do kontroli kamery i skali
    float cameraDistance = 25.0f;
    float cameraAngle = 0.0f;
    float modelScale = 1.0f;
    float minCameraDistance = 4.0f;
    float minModelScale = 0.1f;

    float cameraSpeed = 0.2f;
    float orbitSpeed = 0.015f;
    float scaleSpeed = 0.01f;

    // Wczytujemy teksturę lodu z assets/materials/ice.jpg
    unsigned int iceTexture = loadTexture("assets/materials/iceee.jpg");

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraDistance -= cameraSpeed;
            if (cameraDistance < minCameraDistance) cameraDistance = minCameraDistance;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraDistance += cameraSpeed;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraAngle -= orbitSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraAngle += orbitSpeed;

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) modelScale += scaleSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            modelScale -= scaleSpeed;
            if (modelScale < minModelScale) modelScale = minModelScale;
        }

        glClearColor(0.1f, 0.12f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float camX = sin(cameraAngle) * cameraDistance;
        float camZ = cos(cameraAngle) * cameraDistance;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 2880.0f / 1620.0f, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(camX, 6.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Pozycja i kolor światła ze struktury Scene
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(scene.lights[0].position));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(scene.lights[0].color));

        // --- Rysowanie Latarni (bez tekstury z pliku, używa kolorów z MTL/Wierzchołków) ---
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), GL_FALSE);

        glm::mat4 modelLantern = glm::mat4(1.0f);
        modelLantern = glm::translate(modelLantern, glm::vec3(0.0f, -2.0f, 0.0f));
        modelLantern = glm::scale(modelLantern, glm::vec3(modelScale));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLantern));

        scene.meshes[0].draw();

        // --- Rysowanie Płaszczyzny Lodu (z teksturą ice.jpg) ---
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), GL_TRUE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, iceTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture_diffuse"), 0);

        glm::mat4 modelIce = glm::mat4(1.0f);
        modelIce = glm::translate(modelIce, glm::vec3(0.0f, -2.2f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelIce));

        scene.meshes[1].draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}