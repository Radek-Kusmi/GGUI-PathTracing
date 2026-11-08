#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords; // Location 1 to TexCoords!
layout (location = 2) in vec3 aNormal;    // Location 2 to Normal!
layout (location = 3) in vec3 aColor;

out vec3 Normal;
out vec3 FragPos;
out vec3 VertexColor;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    VertexColor = aColor;
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}