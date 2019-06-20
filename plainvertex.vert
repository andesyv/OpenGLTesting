#version 330 core

layout(location = 0) in vec4 posAttr;
layout(location = 1) in vec4 colAttr;
out vec4 col;
uniform mat4 mMatrix;
layout (std140) uniform globalData {
    mat4 vMatrix;
    mat4 pMatrix;
};

void main() {
   col = colAttr;
   gl_Position = pMatrix * vMatrix * mMatrix * posAttr;
}
