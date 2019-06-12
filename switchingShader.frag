#version 330 core

in vec2 UV;
in vec4 col;

uniform sampler2D textureSampler;
uniform ivec2 windowSize = ivec2(800, 600);
uniform float threshold = 1;

float near = 1.0;
float far  = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

out vec4 fragColor;

void main() {
    if (gl_FragCoord.x / float(windowSize.x) < threshold) {
        fragColor = texture(textureSampler, UV);
    } else {
        float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
        fragColor = vec4(vec3(1 - depth), 1);
    }
}
