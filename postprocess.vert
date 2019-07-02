#version 330 core

layout (location = 0) in vec2 posIn;
layout (location = 1) in vec2 uvIn;

// out vec3 normal;
out vec2 uv;

void main() {
    uv = uvIn;
    // normal = vec3(0, 0, 1);

    gl_Position = vec4(vec3(posIn, 0.0), 1);
}
