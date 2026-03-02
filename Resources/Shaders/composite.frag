#version 450

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D cloudATexture; // 降采样后的云纹理
layout(binding = 1) uniform sampler2D clouddepthATexture;
layout(binding = 2) uniform sampler2D cloudBTexture;
layout(binding = 3) uniform sampler2D clouddepthBTexture;
layout(binding = 4) uniform sampler2D skyboxTexture; 

// 推送常量
layout(push_constant) uniform CompositeParams {
    vec2 fullResolution;      // 全分辨率
    vec2 halfResolution;      // 半分辨率
    float upsampleScale;      // 上采样比例
    float time;               // 时间
    float padding[2]; 
    float framenumber;
} params;

float GaussianKernel[25] = {
    0.00297,   0.01331,   0.02194,   0.01331,   0.00297,   
    0.01331,   0.05963,   0.09832,   0.05963,   0.01331,   
    0.02194,   0.09832,   0.16210,   0.09832,   0.02194,   
    0.01331,   0.05963,   0.09832,   0.05963,   0.01331,   
    0.00297,   0.01331,   0.02194,   0.01331,   0.00297 
};

vec4 gaussianBlurA(vec2 uv, float radius, vec2 resolution) {
    vec4 result = vec4(0.0);
    float totalWeight = 0.0;
    
    // 计算单个像素的大小
    vec2 texelSize = 1.0 / resolution;
    
    // 5x5高斯核的偏移量
    int kernelRadius = 2;
    
    // 应用高斯核
    for (int y = -kernelRadius; y <= kernelRadius; y++) {
        for (int x = -kernelRadius; x <= kernelRadius; x++) {
            // 计算当前采样偏移
            vec2 offset = vec2(x, y) * texelSize * radius;
            vec2 sampleUV = uv + offset;
            
            // 计算高斯权重
            int kernelIndex = (y + kernelRadius) * 5 + (x + kernelRadius);
            float weight = GaussianKernel[kernelIndex];
            
            // 采样并累加
            vec4 sampleColor = texture(cloudATexture, sampleUV);
            result += sampleColor * weight;
            totalWeight += weight;
        }
    }
    
    // 归一化
    if (totalWeight > 0.0) {
        result /= totalWeight;
    }
    
    return result;
}

vec4 gaussianBlurB(vec2 uv, float radius, vec2 resolution) {
    vec4 result = vec4(0.0);
    float totalWeight = 0.0;
    
    // 计算单个像素的大小
    vec2 texelSize = 1.0 / resolution;
    
    // 5x5高斯核的偏移量
    int kernelRadius = 2;
    
    // 应用高斯核
    for (int y = -kernelRadius; y <= kernelRadius; y++) {
        for (int x = -kernelRadius; x <= kernelRadius; x++) {
            // 计算当前采样偏移
            vec2 offset = vec2(x, y) * texelSize * radius;
            vec2 sampleUV = uv + offset;
            
            // 计算高斯权重
            int kernelIndex = (y + kernelRadius) * 5 + (x + kernelRadius);
            float weight = GaussianKernel[kernelIndex];
            
            // 采样并累加
            vec4 sampleColor = texture(cloudBTexture, sampleUV);
            result += sampleColor * weight;
            totalWeight += weight;
        }
    }
    
    // 归一化
    if (totalWeight > 0.0) {
        result /= totalWeight;
    }
    
    return result;
}


vec4 bilateralUpsampleWithBlurA(vec2 uv) {
    vec2 texSize = params.halfResolution;
    vec2 texCoord = uv * texSize;
    
    // 计算双线性插值权重
    vec2 f = fract(texCoord - 0.5);
    float w00 = (1.0 - f.x) * (1.0 - f.y);
    float w10 = f.x * (1.0 - f.y);
    float w01 = (1.0 - f.x) * f.y;
    float w11 = f.x * f.y;
    
    // 获取四个采样点坐标
    vec2 base = floor(texCoord - 0.5) + 0.5;
    vec2 uv00 = (base + vec2(0.0, 0.0)) / texSize;
    vec2 uv10 = (base + vec2(1.0, 0.0)) / texSize;
    vec2 uv01 = (base + vec2(0.0, 1.0)) / texSize;
    vec2 uv11 = (base + vec2(1.0, 1.0)) / texSize;
    
    // 对四个采样点分别应用高斯模糊
    vec4 s00 = gaussianBlurA(uv00, 0, params.halfResolution);
    vec4 s10 = gaussianBlurA(uv10, 0, params.halfResolution);
    vec4 s01 = gaussianBlurA(uv01, 0, params.halfResolution);
    vec4 s11 = gaussianBlurA(uv11, 0, params.halfResolution);

    float totalAlpha = s00.a * w00 + s10.a * w10 + s01.a * w01 + s11.a * w11;
    
    // 预乘Alpha的颜色混合
    vec3 color = (s00.rgb * s00.a * w00 + 
                  s10.rgb * s10.a * w10 + 
                  s01.rgb * s01.a * w01 + 
                  s11.rgb * s11.a * w11) / max(0.01, totalAlpha);
    
    return vec4(color, totalAlpha);
}

vec4 bilateralUpsampleWithBlurB(vec2 uv) {
    vec2 texSize = params.halfResolution;
    vec2 texCoord = uv * texSize;
    
    // 计算双线性插值权重
    vec2 f = fract(texCoord - 0.5);
    float w00 = (1.0 - f.x) * (1.0 - f.y);
    float w10 = f.x * (1.0 - f.y);
    float w01 = (1.0 - f.x) * f.y;
    float w11 = f.x * f.y;
    
    // 获取四个采样点坐标
    vec2 base = floor(texCoord - 0.5) + 0.5;
    vec2 uv00 = (base + vec2(0.0, 0.0)) / texSize;
    vec2 uv10 = (base + vec2(1.0, 0.0)) / texSize;
    vec2 uv01 = (base + vec2(0.0, 1.0)) / texSize;
    vec2 uv11 = (base + vec2(1.0, 1.0)) / texSize;
    
    // 对四个采样点分别应用高斯模糊
    vec4 s00 = gaussianBlurB(uv00, 0, params.halfResolution);
    vec4 s10 = gaussianBlurB(uv10, 0, params.halfResolution);
    vec4 s01 = gaussianBlurB(uv01, 0, params.halfResolution);
    vec4 s11 = gaussianBlurB(uv11, 0, params.halfResolution);

    float totalAlpha = s00.a * w00 + s10.a * w10 + s01.a * w01 + s11.a * w11;
    
    // 预乘Alpha的颜色混合
    vec3 color = (s00.rgb * s00.a * w00 + 
                  s10.rgb * s10.a * w10 + 
                  s01.rgb * s01.a * w01 + 
                  s11.rgb * s11.a * w11) / max(0.01, totalAlpha);
    
    return vec4(color, totalAlpha);
}

float getdepthA(vec2 uv){
    vec2 texSize = params.halfResolution;
    vec2 texCoord = uv * texSize;
    
    // 计算双线性插值权重
    vec2 f = fract(texCoord - 0.5);
    float w00 = (1.0 - f.x) * (1.0 - f.y);
    float w10 = f.x * (1.0 - f.y);
    float w01 = (1.0 - f.x) * f.y;
    float w11 = f.x * f.y;
    
    // 获取四个采样点坐标
    vec2 base = floor(texCoord - 0.5) + 0.5;
    vec2 uv00 = (base + vec2(0.0, 0.0)) / texSize;
    vec2 uv10 = (base + vec2(1.0, 0.0)) / texSize;
    vec2 uv01 = (base + vec2(0.0, 1.0)) / texSize;
    vec2 uv11 = (base + vec2(1.0, 1.0)) / texSize;

    float d00 = texture(clouddepthATexture, uv00).r;
    float d01 = texture(clouddepthATexture, uv01).r;
    float d10 = texture(clouddepthATexture, uv10).r;
    float d11 = texture(clouddepthATexture, uv11).r;

    return min(min(d00, d01), min(d10, d11));
}

float getdepthB(vec2 uv){
    vec2 texSize = params.halfResolution;
    vec2 texCoord = uv * texSize;
    
    // 计算双线性插值权重
    vec2 f = fract(texCoord - 0.5);
    float w00 = (1.0 - f.x) * (1.0 - f.y);
    float w10 = f.x * (1.0 - f.y);
    float w01 = (1.0 - f.x) * f.y;
    float w11 = f.x * f.y;
    
    // 获取四个采样点坐标
    vec2 base = floor(texCoord - 0.5) + 0.5;
    vec2 uv00 = (base + vec2(0.0, 0.0)) / texSize;
    vec2 uv10 = (base + vec2(1.0, 0.0)) / texSize;
    vec2 uv01 = (base + vec2(0.0, 1.0)) / texSize;
    vec2 uv11 = (base + vec2(1.0, 1.0)) / texSize;

    float d00 = texture(clouddepthBTexture, uv00).r;
    float d01 = texture(clouddepthBTexture, uv01).r;
    float d10 = texture(clouddepthBTexture, uv10).r;
    float d11 = texture(clouddepthBTexture, uv11).r;

    return min(min(d00, d01), min(d10, d11));
}

void main() {

    bool isevenframe = (mod(params.framenumber, 2.0)<1.0);

    vec4 cloudColorA = bilateralUpsampleWithBlurA(inUV);
    vec4 cloudColorB = bilateralUpsampleWithBlurB(inUV);

    vec4 cloudColor =vec4(0.0);
    float alpha=1.0;
    float depthex = 0.0;
    if(isevenframe)
    {
        depthex = abs(getdepthB(inUV) - getdepthA(inUV));
        cloudColor = cloudColorA * (1.0-alpha) + cloudColorB * alpha; //混合
        cloudColor = cloudColor * (1.0 - step(0.1, depthex)) + cloudColorB *  step(0.1, depthex); //根据深度截断
    }
    else
    {
        depthex = abs(getdepthA(inUV) - getdepthB(inUV));
        cloudColor = cloudColorB * (1.0-alpha) + cloudColorA * alpha;
        cloudColor = cloudColor * (1.0 - step(0.1, depthex)) + cloudColorA *  step(0.1, depthex);
    }
    


    vec4 skyboxColor = texture(skyboxTexture, inUV);

    skyboxColor.rgb = skyboxColor.rgb * (1.0 - cloudColor.a) + cloudColor.rgb * cloudColor.a;

    outColor = skyboxColor;
}