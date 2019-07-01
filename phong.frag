#version 330 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
} fs_in;

uniform vec3 viewPos;
uniform vec3 lightPos;
vec3 lightColor = vec3(1.0, 0.96, 0.91);
vec3 objectColor = vec3(1, 0, 0);

out vec4 fragColor;

void main() {
    // Ambient
    vec3 ambient = lightColor * 0.15;

    // Diffuse
    vec3 lightDir = normalize(lightPos - fs_in.fragPos);
    vec3 diffuse = lightColor * max(dot(fs_in.normal, lightDir), 0) * 0.8;

    // Specular
    vec3 viewDir = normalize(viewPos - fs_in.fragPos);
    vec3 reflectDir = reflect(-lightDir, fs_in.normal);
    vec3 specular = lightColor * pow(max(dot(viewDir, reflectDir), 0.0), 16) * 0.4;

    fragColor = vec4((ambient + diffuse + specular) * objectColor, 1);
}
