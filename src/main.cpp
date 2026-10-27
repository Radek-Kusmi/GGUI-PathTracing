#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#include <iostream>
#include <vector>
#include <cmath> // Wymagane dla funkcji sin() i cos()
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Scene/Mesh.h"

// Kod Shadera Wierzchołków (Vertex Shader)
const char* vertexShaderSource =
R"(    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    out vec3 Normal;
    out vec3 FragPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    })";

// Kod Shadera Pikseli (Fragment Shader)
const char* fragmentShaderSource =
R"(    #version 330 core
    out vec4 FragColor;
    in vec3 Normal;
    in vec3 FragPos;
    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    void main() {
        // Otoczenie (Ambient)
        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * lightColor;  
        // Oświetlenie dyfuzyjne (Diffuse)
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;        
        vec3 result = (ambient + diffuse) * objectColor;
        FragColor = vec4(result, 1.0);
    })";

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

    Mesh lantern;
    if (!lantern.loadFromFile("assets/models/lightHouse.obj")) {
        std::cerr << "Nie udało się wczytać modelu." << std::endl;
        return -1;
    }

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

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, lantern.vertices.size() * sizeof(Vertex), lantern.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lantern.indices.size() * sizeof(unsigned int), lantern.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);

    // Zmienne do kontrolowania kamery i skali modelu
    float cameraDistance = 15.0f;
    float cameraAngle = 0.0f;        // Zmienna przechowująca kąt obrotu wokół modelu
    float modelScale = 1.0f;
    float minCameraDistance = 4.0f; // Minimalna odległość kamery od środka modelu
	float minModelScale = 0.1f;     // Minimalna skala modelu

    // Prędkości zmian
    float cameraSpeed = 0.1f;
    float orbitSpeed = 0.01f;
    float scaleSpeed = 0.01f;

    while (!glfwWindowShouldClose(window)) {
        // --- OBSŁUGA KLAWIATURY ---
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Zmiana dystansu kamery (W / S) z zabezpieczeniem
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraDistance -= cameraSpeed;
            if (cameraDistance < minCameraDistance) {
                cameraDistance = minCameraDistance; // Blokada przed wejściem w model
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraDistance += cameraSpeed;
        }

        // Orbitowanie wokół modelu (A / D)
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cameraAngle -= orbitSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cameraAngle += orbitSpeed;
        }

        // Zmiana skali modelu (Strzałka Góra / Dół)
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            modelScale += scaleSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            modelScale -= scaleSpeed;
            if (modelScale < minModelScale) {
				modelScale = minModelScale; // Blokada wielkości modelu przed zniknięciem
            }
        }

        glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Obliczanie pozycji kamery na podstawie kąta i odległości
        float camX = sin(cameraAngle) * cameraDistance;
        float camZ = cos(cameraAngle) * cameraDistance;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 2880.0f / 1620.0f, 0.1f, 1000.0f);

        // Kamera patrzy z nowej rotującej pozycji na środek (0, 0, 0)
        glm::mat4 view = glm::lookAt(glm::vec3(camX, 5.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(modelScale));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 5.0f, 10.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.7f, 0.6f, 0.5f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(lantern.indices.size()), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}