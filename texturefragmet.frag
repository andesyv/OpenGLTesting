#version 330 core

in vec2 UV;
uniform sampler2D textureSampler;
uniform float colorAmount = 1.0;
out vec3 fragColor;

void main() {
    fragColor = colorAmount * texture(textureSampler, UV).rgb;
}
