#version 330 core

in vec2 UV;
in vec3 normal;
in vec3 fragPos;

uniform vec3 cameraPos;
uniform samplerCube skybox;

out vec4 fragColor;

void main() {
    vec3 I = normalize(fragPos + cameraPos);
    vec3 R = reflect(I, normalize(normal));
    fragColor = vec4(texture(skybox, R).rgb, 1);
}
