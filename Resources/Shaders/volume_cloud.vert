#version 450 core

const vec3 cubeVertices[36] = vec3[36](
    vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5), vec3(-0.5,  0.5,  0.5),
    vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5),

    vec3(-0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5), vec3( 0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5), vec3( 0.5,  0.5, -0.5),

    vec3(-0.5, -0.5, -0.5), vec3(-0.5, -0.5,  0.5), vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5,  0.5), vec3(-0.5,  0.5,  0.5), vec3(-0.5,  0.5, -0.5),

    vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5, -0.5,  0.5),
    vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5),

    vec3(-0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5, -0.5),
    vec3( 0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5,  0.5),

    vec3(-0.5, -0.5, -0.5), vec3( 0.5, -0.5, -0.5), vec3(-0.5, -0.5,  0.5),
    vec3( 0.5, -0.5, -0.5), vec3( 0.5, -0.5,  0.5), vec3(-0.5, -0.5,  0.5)
);

layout(location = 0) out vec3 fragPosition;


layout(set = 0, binding = 0) uniform UniformBufferVolumeCloud
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 center;
    vec4 size;
} ubo;



void main() {

    vec3 vertex = cubeVertices[gl_VertexIndex];

    vec4 worldPos =  vec4(vertex , 1.0);
    fragPosition = (ubo.model *worldPos).xyz;
    
    gl_Position = ubo.proj * ubo.view * ubo.model * worldPos;
}