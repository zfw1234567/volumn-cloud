#version 450

layout(binding = 1) uniform samplerCube skycubeSampler;

layout(location = 0) in vec3 inTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 color=texture(skycubeSampler, inTexCoord).xyz;
    color = pow(color, vec3(0.4545));
    outColor = vec4(color,1.0);
}
