#version 330 core

in vec4 fragPos;

uniform vec3 lightPos;
uniform float far_plane = 100.f;

out vec4 fragColor;

void main() {
    float lightDistance = length(fragPos.xyz - lightPos);

    lightDistance /= far_plane;

    gl_FragDepth = lightDistance;
}
