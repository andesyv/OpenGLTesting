#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 pMatrix;
uniform mat4 vMatrix;

void main()
{
    TexCoords = aPos;
    mat4 viewMat = mat4(mat3(vMatrix));
    gl_Position = (pMatrix * viewMat * vec4(aPos, 1.0)).xyww;
}
