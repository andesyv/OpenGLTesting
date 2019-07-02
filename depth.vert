#version 330 core

layout (location = 0) in vec3 posIn;

uniform mat4 lightViewProjMatrix;
uniform mat4 mMatrix;

void main() {
    gl_Position = lightViewProjMatrix * mMatrix * vec4(posIn, 1);
}
