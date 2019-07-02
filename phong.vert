#version 330 core

layout (location = 0) in vec3 posIn;
layout (location = 1) in vec3 normIn;
layout (location = 2) in vec2 uvIn;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;
uniform mat4 lightScreenSpaceMatrix;

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
    vec4 lightSpaceFragPos;
} vs_out;

void main() {
    vs_out.normal = mat3(transpose(inverse(mMatrix))) * normIn;
    vs_out.fragPos = vec3(mMatrix * vec4(posIn, 1));
    vs_out.uv = uvIn;
    vs_out.lightSpaceFragPos = lightScreenSpaceMatrix * vec4(vs_out.fragPos, 1);

    gl_Position = pMatrix * vMatrix * vec4(vs_out.fragPos, 1);
}
