#version 330 core

in vec2 UV;
in vec3 normal;
in vec3 fragPos;

uniform vec3 cameraPos;
uniform samplerCube skybox;

out vec4 fragColor;

void main() {
    float ratio = 1.00 / 1.52; // refraction ratio from air (1.00) to glass (1.52)
    vec3 I = normalize(fragPos + cameraPos);
    vec3 R = refract(I, normalize(normal), ratio);
    fragColor = vec4(texture(skybox, R).rgb, 1);
}
