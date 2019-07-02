#version 330 core

layout (location = 0) in vec2 posIn;
layout (location = 1) in vec2 uvIn;

out vec4 col;
out vec2 UV;

void main() {
    UV = uvIn;
    col = vec4(0);

    gl_Position = vec4(vec3(posIn, 0.0), 1);
}
