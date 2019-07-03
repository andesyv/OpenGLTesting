#version 330 core

in vec3 TexCoords;

uniform samplerCube skybox;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vec3(texture(skybox, TexCoords).r), 1);
}
