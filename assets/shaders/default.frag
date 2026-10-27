#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 VertexColor; // Kolor odebrany z Vertex Shadera

uniform vec3 lightPos;
uniform vec3 lightColor;

void main() {
    // Otoczenie (Ambient)
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;  
    
    // Oświetlenie dyfuzyjne (Diffuse)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;        
    
    // Wynik z użyciem wczytanego koloru VertexColor
    vec3 result = (ambient + diffuse) * VertexColor;
    FragColor = vec4(result, 1.0);
}