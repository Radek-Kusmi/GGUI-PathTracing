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

#include "Scene/Mesh.h"
#include "Utils/ShaderLoader.h"

int main() {
    if (!glfwInit()) {
        std::cerr << "Błąd inicjalizacji GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(2880, 1620, "Scena 3D - Latarnia", NULL, NULL);
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

    // Wczytanie kodów shaderów
    std::string vertCode = loadShaderSource("assets/shaders/default.vert");
    std::string fragCode = loadShaderSource("assets/shaders/default.frag");
    const char* vertexShaderSource = vertCode.c_str();
    const char* fragmentShaderSource = fragCode.c_str();

    Mesh lantern;
    if (!lantern.loadFromOBJ("assets/models/Lighthouse1/lightHouse.obj")) {
        std::cerr << "Nie udało się wczytać modelu." << std::endl;
        return -1;
    }

    // Kompilacja shaderów
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

    // Zmienne do kontrolowania kamery i skali modelu
    float cameraDistance = 15.0f;
    float cameraAngle = 0.0f;
    float modelScale = 1.0f;
    float minCameraDistance = 4.0f;
    float minModelScale = 0.1f;

    float cameraSpeed = 0.1f;
    float orbitSpeed = 0.01f;
    float scaleSpeed = 0.01f;

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

        glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float camX = sin(cameraAngle) * cameraDistance;
        float camZ = cos(cameraAngle) * cameraDistance;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 2880.0f / 1620.0f, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(camX, 5.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(modelScale));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 5.0f, 10.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        // Używamy rysowania z obiektu klasy Mesh, który zarządza własnym VAO
        lantern.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}