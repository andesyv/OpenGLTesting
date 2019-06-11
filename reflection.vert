#version 330 core

layout(location = 0) in vec3 posAttr;
layout(location = 1) in vec3 normAttr;
layout(location = 2) in vec2 vertexUV;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

out vec3 normal;
out vec2 UV;
out vec3 fragPos;

void main() {
   normal = mat3(transpose(inverse(mMatrix))) * normAttr;
   UV = vertexUV;
   fragPos = vec3(mMatrix * vec4(posAttr, 1));
   gl_Position = pMatrix * vMatrix * vec4(fragPos, 1);
}
