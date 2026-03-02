#ifndef VOLUMETEXTUREGENERATOR_H
#define VOLUMETEXTUREGENERATOR_H
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <vulkanDataStructure/tool/perlinnoise.h>
class VolumeTextureGenerator {
private:
    // 频率倍数数组
    float frequenceMul[6] = {2.0f, 8.0f, 14.0f, 20.0f, 26.0f, 32.0f};

    glm::vec4 mod289(const glm::vec4 &x)
    {
        return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f;
    }

    glm::vec4 permute(const glm::vec4 &x)
    {
        return mod289(((x * 34.0f) + 1.0f) * x);
    }

    glm::vec4 taylorInvSqrt(const glm::vec4 &r)
    {
        return glm::vec4(1.79284291400159f) - glm::vec4(0.85373472095314f) * r;
    }

    glm::vec4 fade(const glm::vec4 &t)
    {
        return (t * t * t) * (t * (t * 6.0f - glm::vec4(15.0f)) + glm::vec4(10.0f));
    }

    // 从GLM移植的4D Perlin噪声函数
    float glmPerlin4D(const glm::vec4& position, const glm::vec4& rep) {

    glm::vec4 Pi0 = glm::mod(glm::floor(position), rep);    // 整数部分用于索引
    glm::vec4 Pi1 = glm::mod(Pi0 + glm::vec4(1.0f), rep);   // 整数部分 + 1
    glm::vec4 Pf0 = glm::fract(position);                   // 插值的小数部分
    glm::vec4 Pf1 = Pf0 - glm::vec4(1.0f);                  // 小数部分 - 1.0
    
    // 构建网格顶点索引
    glm::vec4 ix = glm::vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    glm::vec4 iy = glm::vec4(Pi0.y, Pi0.y, Pi1.y, Pi1.y);
    glm::vec4 iz0 = glm::vec4(Pi0.z);
    glm::vec4 iz1 = glm::vec4(Pi1.z);
    glm::vec4 iw0 = glm::vec4(Pi0.w);
    glm::vec4 iw1 = glm::vec4(Pi1.w);
    
    // 排列组合生成梯度表索引
    glm::vec4 ixy = permute(permute(ix) + iy);
    glm::vec4 ixy0 = permute(ixy + iz0);
    glm::vec4 ixy1 = permute(ixy + iz1);
    glm::vec4 ixy00 = permute(ixy0 + iw0);
    glm::vec4 ixy01 = permute(ixy0 + iw1);
    glm::vec4 ixy10 = permute(ixy1 + iw0);
    glm::vec4 ixy11 = permute(ixy1 + iw1);
    
    // 生成16个梯度向量（4D）
    glm::vec4 gx00 = ixy00 / 7.0f;
    glm::vec4 gy00 = glm::floor(gx00) / 7.0f;
    glm::vec4 gz00 = glm::floor(gy00) / 6.0f;
    gx00 = glm::fract(gx00) - glm::vec4(0.5f);
    gy00 = glm::fract(gy00) - glm::vec4(0.5f);
    gz00 = glm::fract(gz00) - glm::vec4(0.5f);
    glm::vec4 gw00 = glm::vec4(0.75f) - glm::abs(gx00) - glm::abs(gy00) - glm::abs(gz00);
    glm::vec4 sw00 = glm::step(gw00, glm::vec4(0.0f));
    gx00 -= sw00 * (glm::step(glm::vec4(0.0f), gx00) - glm::vec4(0.5f));
    gy00 -= sw00 * (glm::step(glm::vec4(0.0f), gy00) - glm::vec4(0.5f));
    
    glm::vec4 gx01 = ixy01 / 7.0f;
    glm::vec4 gy01 = glm::floor(gx01) / 7.0f;
    glm::vec4 gz01 = glm::floor(gy01) / 6.0f;
    gx01 = glm::fract(gx01) - glm::vec4(0.5f);
    gy01 = glm::fract(gy01) - glm::vec4(0.5f);
    gz01 = glm::fract(gz01) - glm::vec4(0.5f);
    glm::vec4 gw01 = glm::vec4(0.75f) - glm::abs(gx01) - glm::abs(gy01) - glm::abs(gz01);
    glm::vec4 sw01 = glm::step(gw01, glm::vec4(0.0f));
    gx01 -= sw01 * (glm::step(glm::vec4(0.0f), gx01) - glm::vec4(0.5f));
    gy01 -= sw01 * (glm::step(glm::vec4(0.0f), gy01) - glm::vec4(0.5f));
    
    glm::vec4 gx10 = ixy10 / 7.0f;
    glm::vec4 gy10 = glm::floor(gx10) / 7.0f;
    glm::vec4 gz10 = glm::floor(gy10) / 6.0f;
    gx10 = glm::fract(gx10) - glm::vec4(0.5f);
    gy10 = glm::fract(gy10) - glm::vec4(0.5f);
    gz10 = glm::fract(gz10) - glm::vec4(0.5f);
    glm::vec4 gw10 = glm::vec4(0.75f) - glm::abs(gx10) - glm::abs(gy10) - glm::abs(gz10);
    glm::vec4 sw10 = glm::step(gw10, glm::vec4(0.0f));
    gx10 -= sw10 * (glm::step(glm::vec4(0.0f), gx10) - glm::vec4(0.5f));
    gy10 -= sw10 * (glm::step(glm::vec4(0.0f), gy10) - glm::vec4(0.5f));
    
    glm::vec4 gx11 = ixy11 / 7.0f;
    glm::vec4 gy11 = glm::floor(gx11) / 7.0f;
    glm::vec4 gz11 = glm::floor(gy11) / 6.0f;
    gx11 = glm::fract(gx11) - glm::vec4(0.5f);
    gy11 = glm::fract(gy11) - glm::vec4(0.5f);
    gz11 = glm::fract(gz11) - glm::vec4(0.5f);
    glm::vec4 gw11 = glm::vec4(0.75f) - glm::abs(gx11) - glm::abs(gy11) - glm::abs(gz11);
    glm::vec4 sw11 = glm::step(gw11, glm::vec4(0.0f));
    gx11 -= sw11 * (glm::step(glm::vec4(0.0f), gx11) - glm::vec4(0.5f));
    gy11 -= sw11 * (glm::step(glm::vec4(0.0f), gy11) - glm::vec4(0.5f));
    
    // 重组为梯度向量
    glm::vec4 g0000 = glm::vec4(gx00.x, gy00.x, gz00.x, gw00.x);
    glm::vec4 g1000 = glm::vec4(gx00.y, gy00.y, gz00.y, gw00.y);
    glm::vec4 g0100 = glm::vec4(gx00.z, gy00.z, gz00.z, gw00.z);
    glm::vec4 g1100 = glm::vec4(gx00.w, gy00.w, gz00.w, gw00.w);
    glm::vec4 g0010 = glm::vec4(gx10.x, gy10.x, gz10.x, gw10.x);
    glm::vec4 g1010 = glm::vec4(gx10.y, gy10.y, gz10.y, gw10.y);
    glm::vec4 g0110 = glm::vec4(gx10.z, gy10.z, gz10.z, gw10.z);
    glm::vec4 g1110 = glm::vec4(gx10.w, gy10.w, gz10.w, gw10.w);
    glm::vec4 g0001 = glm::vec4(gx01.x, gy01.x, gz01.x, gw01.x);
    glm::vec4 g1001 = glm::vec4(gx01.y, gy01.y, gz01.y, gw01.y);
    glm::vec4 g0101 = glm::vec4(gx01.z, gy01.z, gz01.z, gw01.z);
    glm::vec4 g1101 = glm::vec4(gx01.w, gy01.w, gz01.w, gw01.w);
    glm::vec4 g0011 = glm::vec4(gx11.x, gy11.x, gz11.x, gw11.x);
    glm::vec4 g1011 = glm::vec4(gx11.y, gy11.y, gz11.y, gw11.y);
    glm::vec4 g0111 = glm::vec4(gx11.z, gy11.z, gz11.z, gw11.z);
    glm::vec4 g1111 = glm::vec4(gx11.w, gy11.w, gz11.w, gw11.w);
    
    // 归一化梯度向量
    glm::vec4 norm00 = taylorInvSqrt(glm::vec4(
        glm::dot(g0000, g0000),
        glm::dot(g0100, g0100),
        glm::dot(g1000, g1000),
        glm::dot(g1100, g1100)
    ));
    g0000 *= norm00.x;
    g0100 *= norm00.y;
    g1000 *= norm00.z;
    g1100 *= norm00.w;
    
    glm::vec4 norm01 = taylorInvSqrt(glm::vec4(
        glm::dot(g0001, g0001),
        glm::dot(g0101, g0101),
        glm::dot(g1001, g1001),
        glm::dot(g1101, g1101)
    ));
    g0001 *= norm01.x;
    g0101 *= norm01.y;
    g1001 *= norm01.z;
    g1101 *= norm01.w;
    
    glm::vec4 norm10 = taylorInvSqrt(glm::vec4(
        glm::dot(g0010, g0010),
        glm::dot(g0110, g0110),
        glm::dot(g1010, g1010),
        glm::dot(g1110, g1110)
    ));
    g0010 *= norm10.x;
    g0110 *= norm10.y;
    g1010 *= norm10.z;
    g1110 *= norm10.w;
    
    glm::vec4 norm11 = taylorInvSqrt(glm::vec4(
        glm::dot(g0011, g0011),
        glm::dot(g0111, g0111),
        glm::dot(g1011, g1011),
        glm::dot(g1111, g1111)
    ));
    g0011 *= norm11.x;
    g0111 *= norm11.y;
    g1011 *= norm11.z;
    g1111 *= norm11.w;
    
    // 计算16个顶点的贡献值
    float n0000 = glm::dot(g0000, Pf0);
    float n1000 = glm::dot(g1000, glm::vec4(Pf1.x, Pf0.y, Pf0.z, Pf0.w));
    float n0100 = glm::dot(g0100, glm::vec4(Pf0.x, Pf1.y, Pf0.z, Pf0.w));
    float n1100 = glm::dot(g1100, glm::vec4(Pf1.x, Pf1.y, Pf0.z, Pf0.w));
    float n0010 = glm::dot(g0010, glm::vec4(Pf0.x, Pf0.y, Pf1.z, Pf0.w));
    float n1010 = glm::dot(g1010, glm::vec4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
    float n0110 = glm::dot(g0110, glm::vec4(Pf0.x, Pf1.y, Pf1.z, Pf0.w));
    float n1110 = glm::dot(g1110, glm::vec4(Pf1.x, Pf1.y, Pf1.z, Pf0.w));
    float n0001 = glm::dot(g0001, glm::vec4(Pf0.x, Pf0.y, Pf0.z, Pf1.w));
    float n1001 = glm::dot(g1001, glm::vec4(Pf1.x, Pf0.y, Pf0.z, Pf1.w));
    float n0101 = glm::dot(g0101, glm::vec4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
    float n1101 = glm::dot(g1101, glm::vec4(Pf1.x, Pf1.y, Pf0.z, Pf1.w));
    float n0011 = glm::dot(g0011, glm::vec4(Pf0.x, Pf0.y, Pf1.z, Pf1.w));
    float n1011 = glm::dot(g1011, glm::vec4(Pf1.x, Pf0.y, Pf1.z, Pf1.w));
    float n0111 = glm::dot(g0111, glm::vec4(Pf0.x, Pf1.y, Pf1.z, Pf1.w));
    float n1111 = glm::dot(g1111, Pf1);
    
    // 4D插值
    glm::vec4 fade_xyzw = fade(Pf0);
    glm::vec4 n_0w = glm::mix(
        glm::vec4(n0000, n1000, n0100, n1100),
        glm::vec4(n0001, n1001, n0101, n1101),
        fade_xyzw.w
    );
    glm::vec4 n_1w = glm::mix(
        glm::vec4(n0010, n1010, n0110, n1110),
        glm::vec4(n0011, n1011, n0111, n1111),
        fade_xyzw.w
    );
    glm::vec4 n_zw = glm::mix(n_0w, n_1w, fade_xyzw.z);
    glm::vec2 n_yzw = glm::mix(
        glm::vec2(n_zw.x, n_zw.y),
        glm::vec2(n_zw.z, n_zw.w),
        fade_xyzw.y
    );
    float n_xyzw = glm::mix(n_yzw.x, n_yzw.y, fade_xyzw.x);
    
    return 2.2f * n_xyzw;
    }
    
    // 简化版的hash函数
    float hash(int n) {
        return glm::fract(sinf((float)n + 1.951f) * 43758.5453123f);
    }

    glm::vec3 hash33(glm::vec3 p3)
    {
        p3 = glm::fract(p3 * glm::vec3(.1031, .1030, .0973));
        p3 += glm::dot(p3, glm::vec3(p3.y+ 33.33,p3.x+ 33.33,p3.z+ 33.33));
        return glm::fract(( glm::vec3(p3.x,p3.x,p3.y) + glm::vec3(p3.y,p3.x,p3.x)) * glm::vec3(p3.z,p3.y,p3.x));
    }

    // 3D噪声函数
    float noise(const glm::vec3& x) {
        glm::vec3 p = glm::floor(x);
        glm::vec3 f = glm::fract(x);
        
        f = f * f * (glm::vec3(3.0f) - glm::vec3(2.0f) * f);
        float n = p.x + p.y * 57.0f + 113.0f * p.z;
        
        return glm::mix(
            glm::mix(
                glm::mix(hash((int)n + 0), hash((int)n + 1), f.x),
                glm::mix(hash((int)n + 57), hash((int)n + 58), f.x),
                f.y),
            glm::mix(
                glm::mix(hash((int)n + 113), hash((int)n + 114), f.x),
                glm::mix(hash((int)n + 170), hash((int)n + 171), f.x),
                f.y),
            f.z);
    }
    
    // Worley噪声函数
    float worleyNoise3D(const glm::vec3& p, float cellCount) {
        // 1. 将坐标映射到单元格空间
    glm::vec3 pCell = p * cellCount;
    
    // 2. 找到当前点所在的整数单元格
    glm::ivec3 baseCell = glm::ivec3(glm::floor(pCell));
    
    // 3. 初始化最小距离为最大值
    float minDist = 1.0e10f;
    
    // 4. 遍历当前单元格及其周围26个相邻单元格
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                // 获取相邻单元格坐标
                glm::ivec3 neighborCell = baseCell + glm::ivec3(dx, dy, dz);
                
                // 为每个单元格生成随机特征点（在0-1范围内）
                glm::vec3 randomOffset = hash33(neighborCell);
                
                // 计算特征点的绝对位置
                glm::vec3 featurePoint = glm::vec3(neighborCell) + randomOffset;
                
                // 计算当前点到特征点的向量
                glm::vec3 delta = pCell - featurePoint;
                
                // 计算距离的平方（比实际距离计算快）
                float distSq = glm::dot(delta, delta);
                
                // 更新最小距离
                if (distSq < minDist) {
                    minDist = distSq;
                }
            }
        }
    }
    
    // 5. 返回最近距离（实际距离，而非平方）
    return glm::sqrt(minDist);
    }
    
    // 3D Perlin噪声（使用4D实现避免Z轴条纹）
    float perlinNoise3D(const glm::vec3& pIn, float frequency, int octaveCount) {
        

        float octaveFrequencyFactor = 2.0f;
        float sum = 0.0f;
        float weightSum = 0.0f;
        float weight = 0.5f;
        float currentFreq = frequency;

        

        currentFreq = frequency;
        for (int oct = 0; oct < octaveCount; oct++) {
            glm::vec4 p = glm::vec4(pIn.x, pIn.y, pIn.z, 0.0f) * currentFreq;
            float val = this->perlin[oct].generateNoise(pIn);
            
            sum += val * weight;
            weightSum += weight;
            
            weight *= weight;
            currentFreq *= octaveFrequencyFactor;
        }
        
        float noiseVal = sum / weightSum;
        noiseVal = std::min(noiseVal, 1.0f);
        noiseVal = std::max(noiseVal, 0.0f);
        return noiseVal;
    }
    
    // 重映射函数
    float remap(float originalValue, float originalMin, float originalMax, 
                float newMin, float newMax) {
        return newMin + (((originalValue - originalMin) / 
                         (originalMax - originalMin)) * (newMax - newMin));
    }
    
    // 堆叠3D噪声 - 与着色器中的stackable3DNoise函数对应
    glm::vec4 stackable3DNoise(const glm::vec3& coord) {
        // 1. Perlin FBM噪声
        int octaveCount = 3;
        float frequency = 8.0f;
        float perlinNoise = perlinNoise3D(coord, frequency, octaveCount);
    

        // 2. Perlin-Worley噪声
        float perlinWorleyNoise = 0.0f;
        {
            float cellCount = 4.0f;
            float worleyNoise0 = 1.0f - worleyNoise3D(coord, cellCount * frequenceMul[0]);
            float worleyNoise1 = 1.0f - worleyNoise3D(coord, cellCount * frequenceMul[1]);
            float worleyNoise2 = 1.0f - worleyNoise3D(coord, cellCount * frequenceMul[2]);
            float worleyNoise3 = 1.0f - worleyNoise3D(coord, cellCount * frequenceMul[3]);
            float worleyNoise4 = 1.0f - worleyNoise3D(coord, cellCount * frequenceMul[4]);
            float worleyNoise5 = 1.0f - worleyNoise3D(coord, cellCount * frequenceMul[5]);
            
            // 创建Worley FBM
            float worleyFBM = worleyNoise0 * 0.625f + 
                             worleyNoise1 * 0.25f + 
                             worleyNoise2 * 0.125f;
            
            // 重映射Perlin噪声
            perlinWorleyNoise = remap(perlinNoise, 0.0f, 1.0f, worleyFBM, 1.0f);
            
            // 平方以增加对比度
            perlinWorleyNoise = perlinWorleyNoise * perlinWorleyNoise;
        }

        //perlinWorleyNoise=perlinNoise;

        // 3. 多频率Worley噪声
        float cellCount = 4.0f;
        float worleyNoise0 =std::max(1.0f - worleyNoise3D(coord, cellCount * 1.0f)-0.3,0.0) ;
        float worleyNoise1 = std::max(1.0f - worleyNoise3D(coord, cellCount * 2.0f)-0.3,0.0);
        float worleyNoise2 = std::max(1.0f - worleyNoise3D(coord, cellCount * 4.0f)-0.3,0.0);
        float worleyNoise3 = std::max(1.0f - worleyNoise3D(coord, cellCount * 8.0f)-0.3,0.0);
        float worleyNoise4 = std::max(1.0f - worleyNoise3D(coord, cellCount * 16.0f)-0.3,0.0);
        
        // 三个频率的Worley FBM
        float worleyFBM0 = worleyNoise1 * 0.625f + 
                          worleyNoise2 * 0.25f + 
                          worleyNoise3 * 0.125f;
        
        float worleyFBM1 = worleyNoise2 * 0.625f + 
                          worleyNoise3 * 0.25f + 
                          worleyNoise4 * 0.125f;
        
        float worleyFBM2 = worleyNoise3 * 0.75f + 
                          worleyNoise4 * 0.25f;
        
                
        return glm::vec4(perlinWorleyNoise, worleyFBM0, worleyFBM1, worleyFBM2);
    }
    
public:
    std::vector<perlinnoise>perlin;
    // 主函数 - 生成云密度场
    float generateCloudDensity(float nx, float ny, float nz) {
        glm::vec3 coord = glm::vec3(nx, ny, nz);
        
        // 调用堆叠噪声函数
        glm::vec4 noiseValues = stackable3DNoise(coord);
        
        // 返回R通道的Perlin-Worley噪声作为基础密度
        return noiseValues.r;
    }
    
    // 生成完整的3D纹理（4通道）
    std::vector<glm::vec4> generateCloudTexture3D(int width, int height, int depth) {
        std::vector<glm::vec4> textureData(width * height * depth);
        float octaveCount=3;
        float currentFreq=4;
        float octaveFrequencyFactor=2;
        this->perlin.resize(octaveCount);
        for (int oct = 0; oct < octaveCount; oct++) {
            this->perlin[oct].set3D(currentFreq);
            currentFreq *= octaveFrequencyFactor;
        }

        for (int z = 0; z < depth; z++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    float nx = (float)x / width;
                    float ny = (float)y / height;
                    float nz = (float)z / depth;
                    
                    int idx = z * width * height + y * width + x;
                    textureData[idx] = stackable3DNoise(glm::vec3(nx, ny, nz));
                }
            }
            std::cout<<"密度场生成中 "<<z<<" / "<<depth<<std::endl;
        }

        float minDensity = INFINITY, maxDensity = -INFINITY, avgDensity = 0;
        for (glm::vec4 d : textureData) {
            minDensity = std::min(minDensity, d.x);
            maxDensity = std::max(maxDensity, d.x);
            avgDensity += d.x;
        }
        avgDensity /= textureData.size();
        
        std::cout << "Density range: [" << minDensity << ", " << maxDensity
                  << "], avg: " << avgDensity << std::endl;
        
        return textureData;
    }
    
    // 原始函数，但使用新的噪声生成方法
    std::vector<float> generateCloudTexture3DWithGradient(int width, int height, int depth) {
        std::vector<float> densityField(width * height * depth);
        
        for (int z = 0; z < depth; z++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    float nx = (float)x / width;
                    float ny = (float)y / height;
                    float nz = (float)z / depth;
                    
                    int idx = z * width * height + y * width + x;
                    
                    // 使用着色器中的方法生成密度
                    glm::vec3 coord = glm::vec3(nx, ny, nz);
                    glm::vec4 noiseValues = stackable3DNoise(coord);
                    
                    // 使用Perlin-Worley噪声作为基础密度
                    densityField[idx] = std::min(noiseValues.r, 1.0f);
                }
            }
        }
        
        // 分析密度范围
        float minDensity = INFINITY, maxDensity = -INFINITY, avgDensity = 0;
        for (float d : densityField) {
            minDensity = std::min(minDensity, d);
            maxDensity = std::max(maxDensity, d);
            avgDensity += d;
        }
        avgDensity /= densityField.size();
        
        std::cout << "Density range: [" << minDensity << ", " << maxDensity
                  << "], avg: " << avgDensity << std::endl;
        
        return densityField;
    }
    
    // 可选：生成带梯度的密度场
    struct DensityWithGradient {
        float density;
        glm::vec3 gradient;
    };
    
    std::vector<DensityWithGradient> generateCloudTextureWithGradient(int width, int height, int depth) {
        std::vector<DensityWithGradient> densityField(width * height * depth);
        float eps = 1.0f / std::max(std::max(width, height), depth);
        
        for (int z = 0; z < depth; z++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    float nx = (float)x / width;
                    float ny = (float)y / height;
                    float nz = (float)z / depth;
                    
                    int idx = z * width * height + y * width + x;
                    
                    // 中心密度
                    glm::vec3 coord = glm::vec3(nx, ny, nz);
                    glm::vec4 noiseValues = stackable3DNoise(coord);
                    float density = std::min(noiseValues.r, 1.0f);
                    
                    // 计算梯度（通过中心差分）
                    glm::vec3 dx = glm::vec3(nx + eps, ny, nz);
                    glm::vec3 dy = glm::vec3(nx, ny + eps, nz);
                    glm::vec3 dz = glm::vec3(nx, ny, nz + eps);
                    
                    float densityX = std::min(stackable3DNoise(dx).r, 1.0f);
                    float densityY = std::min(stackable3DNoise(dy).r, 1.0f);
                    float densityZ = std::min(stackable3DNoise(dz).r, 1.0f);
                    
                    glm::vec3 gradient = glm::vec3(
                        (densityX - density) / eps,
                        (densityY - density) / eps,
                        (densityZ - density) / eps
                    );
                    
                    densityField[idx] = {density, gradient};
                }
            }
        }
        
        return densityField;
    }
};
#endif