#version 450 core

// 硬编码的立方体顶点数据
const vec3 cubeVertices[36] = vec3[36](
    // 前平面
    vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5), vec3(-0.5,  0.5,  0.5),
    vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5),
    // 后平面
    vec3(-0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5), vec3( 0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5), vec3( 0.5,  0.5, -0.5),
    // 左平面
    vec3(-0.5, -0.5, -0.5), vec3(-0.5, -0.5,  0.5), vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5,  0.5), vec3(-0.5,  0.5,  0.5), vec3(-0.5,  0.5, -0.5),
    // 右平面
    vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5, -0.5,  0.5),
    vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5),
    // 上平面
    vec3(-0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5, -0.5),
    vec3( 0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5,  0.5),
    // 下平面
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
    // 获取硬编码的顶点
    vec3 vertex = cubeVertices[gl_VertexIndex];
    //vertex=vertex*ubo.size.xyz;
    
    // 转换为世界空间位置
    vec4 worldPos =  vec4(vertex , 1.0);
    fragPosition = (ubo.model *worldPos).xyz;
    
    // 计算裁剪空间位置
    gl_Position = ubo.proj * ubo.view * ubo.model * worldPos;
}