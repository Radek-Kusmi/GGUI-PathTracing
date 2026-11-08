#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 VertexColor;
in vec2 TexCoords; // Przekazane z Vertex Shadera

uniform sampler2D texture_diffuse;
uniform bool useTexture;

uniform vec3 lightPos;
uniform vec3 lightColor;

void main() {
    // 1. Otoczenie (Ambient)
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;  
    
    // 2. Oświetlenie dyfuzyjne (Diffuse)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;        
    
    // 3. Wybór koloru (Tekstura vs VertexColor)
    vec3 objectColor = VertexColor;
    if (useTexture) {
        objectColor = texture(texture_diffuse, TexCoords).rgb;
    }

    // 4. Wynik końcowy
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}