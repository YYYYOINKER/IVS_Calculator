#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 objectColor;
uniform vec3 lightColor;

void main() {
        
        // simple diffuse light
        float diff = max(dot(normalize(Normal), -lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // ambient + diffuse
        vec3 result = (diffuse + vec3(0.1)) * objectColor;
        FragColor = vec4(result, 1.0);
}

