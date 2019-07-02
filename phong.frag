#version 330 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
    vec4 lightSpaceFragPos;
} fs_in;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform sampler2D shadowMap;

vec3 lightColor = vec3(1.0, 0.96, 0.91);
vec3 objectColor = vec3(1, 0, 0);

out vec4 fragColor;

float calculateShadow(vec4 lightScreenSpaceFragPos, vec3 normal, vec3 lightDir) {
    // Perspective divide
    vec3 projCoord = lightScreenSpaceFragPos.xyz / lightScreenSpaceFragPos.w;
    // Convert from range [-1, 1] to range [0, 1]
    projCoord = projCoord * 0.5 + 0.5;

    float currentDepth = projCoord.z;
    if (currentDepth > 1.0)
        return 0.0;

    // Add in a bias to fix shadow acne
    float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.00005);

    // PCF (Percentage-closer filtering)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoord.xy + vec2(x, y) * texelSize).r;
            shadow += float(currentDepth - bias > pcfDepth);
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main() {
    vec3 norm = normalize(fs_in.normal);
    // Ambient
    vec3 ambient = lightColor * 0.15;

    // Diffuse
    vec3 lightDir = normalize(lightPos - fs_in.fragPos);
    vec3 diffuse = lightColor * max(dot(norm, lightDir), 0) * 0.8;

    // Specular
    vec3 viewDir = normalize(fs_in.fragPos - viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 specular = lightColor * pow(max(dot(viewDir, reflectDir), 0.0), 16) * 0.4;

    // Shadows
    float shadowMult = 1.0 - calculateShadow(fs_in.lightSpaceFragPos, norm, lightDir);

    fragColor = vec4((ambient + shadowMult * (diffuse + specular)) * objectColor, 1);
}
