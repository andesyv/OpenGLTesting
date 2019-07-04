#version 330 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
} fs_in;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform samplerCube shadowMap;

uniform float far_plane = 100.f;

vec3 lightColor = vec3(1.0, 0.96, 0.91);
vec3 objectColor = vec3(1, 0, 0);

out vec4 fragColor;

float calculateShadow(vec3 fragPos, vec3 normal, vec3 lightDir) {
    vec3 fragToLight = fragPos - lightPos;

    float currentDepth = length(fragToLight);

    // Add in a bias to fix shadow acne
    float bias = // max(0.0005 * (1.0 - dot(normal, lightDir)), 0.00005); // Uncomment this for a very interesting effect...
    0.15;

    // PCF (Percentage-closer filtering)
    float shadow = 0.0;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskradius = (1.0 + (viewDistance / far_plane)) / 50.0;
    vec3 sampleOffsetDirections[20] = vec3[]
    (
       vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
       vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
       vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
       vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
       vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );

    for (int i = 0; i < samples; ++i) {
        float pcfDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskradius).r;
        pcfDepth *= far_plane;
        shadow += float(currentDepth - bias > pcfDepth);
    }
    shadow /= float(samples);

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
    float shadowMult = 1.0 - calculateShadow(fs_in.fragPos, norm, lightDir);


    fragColor = vec4((ambient + shadowMult * (diffuse + specular)) * objectColor, 1);

    // // Debugging
    // vec3 fragToLight = fs_in.fragPos - lightPos;
    // float closestDepth = texture(shadowMap, fragToLight).r;
    // // closestDepth *= far_plane;
    // fragColor = vec4(vec3(closestDepth), 1.0);
}
