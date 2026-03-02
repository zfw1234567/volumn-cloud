#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class perlinnoise
{
private:
    int cellcount;
    std::vector<glm::vec3> gradients;
    
    // 三次样条插值
    float smoothstep(float t) 
    {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }
    
    // 根据晶格坐标获取梯度
    glm::vec3 getGradient(glm::ivec3 cell) 
    {
        int x = cell.x & (cellcount-1);
        int y = cell.y & (cellcount-1);
        int z = cell.z & (cellcount-1);
        int index = z * (cellcount+1) * (cellcount+1) + y * (cellcount+1) + x;
        return gradients[index];
    }
    
public:
    perlinnoise()
    {

    }
    ~perlinnoise()
    {

    }
    
    glm::vec3 hash33(glm::vec3 p3)
    {
        p3 = glm::fract(p3 * glm::vec3(.1031f, .1030f, .0973f));
        p3 += glm::dot(p3, glm::vec3(p3.y + 33.33f, p3.x + 33.33f, p3.z + 33.33f));
        return glm::fract((glm::vec3(p3.x, p3.x, p3.y) + glm::vec3(p3.y, p3.x, p3.x)) * glm::vec3(p3.z, p3.y, p3.x));
    }

    void set3D(int cellcount)
    {
        this->cellcount = cellcount;
        int gridSize = cellcount + 1;
        this->gradients.reserve(gridSize * gridSize * gridSize);
        for (int k = 0; k < gridSize; k++) {
            for (int j = 0; j < gridSize; j++) {
                for (int i = 0; i < gridSize; i++) {
                    glm::vec3 v = glm::normalize(this->hash33(glm::vec3(i, j + 0.1045f, k + 0.7234f)));
                    // 将哈希值从[0,1]映射到[-1,1]并归一化
                    v = glm::normalize(v * 2.0f - 1.0f);
                    this->gradients.push_back(v);
                }
            }
        }
    }

    float generateNoise(glm::vec3 p)
    {
        // 1. 将百分比坐标映射到晶格空间
        glm::vec3 pos = p * (float)cellcount;
        
        // 2. 找到晶格坐标（整数部分）和小数部分
        glm::ivec3 cell = glm::ivec3(glm::floor(pos));
        glm::vec3 frac = pos - glm::vec3(cell);
        
        // 3. 计算三次样条插值权重
        glm::vec3 smooth = glm::vec3(
            smoothstep(frac.x),
            smoothstep(frac.y),
            smoothstep(frac.z)
        );
        
        // 4. 获取8个晶格顶点的梯度向量
        glm::vec3 grad000 = getGradient(cell);
        glm::vec3 grad100 = getGradient(cell + glm::ivec3(1, 0, 0));
        glm::vec3 grad010 = getGradient(cell + glm::ivec3(0, 1, 0));
        glm::vec3 grad110 = getGradient(cell + glm::ivec3(1, 1, 0));
        glm::vec3 grad001 = getGradient(cell + glm::ivec3(0, 0, 1));
        glm::vec3 grad101 = getGradient(cell + glm::ivec3(1, 0, 1));
        glm::vec3 grad011 = getGradient(cell + glm::ivec3(0, 1, 1));
        glm::vec3 grad111 = getGradient(cell + glm::ivec3(1, 1, 1));
        
        // 5. 计算到8个顶点的距离向量
        glm::vec3 d000 = frac;
        glm::vec3 d100 = frac - glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 d010 = frac - glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 d110 = frac - glm::vec3(1.0f, 1.0f, 0.0f);
        glm::vec3 d001 = frac - glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 d101 = frac - glm::vec3(1.0f, 0.0f, 1.0f);
        glm::vec3 d011 = frac - glm::vec3(0.0f, 1.0f, 1.0f);
        glm::vec3 d111 = frac - glm::vec3(1.0f, 1.0f, 1.0f);
        
        // 6. 计算每个顶点的点积（梯度向量·距离向量）
        float dot000 = glm::dot(grad000, d000);
        float dot100 = glm::dot(grad100, d100);
        float dot010 = glm::dot(grad010, d010);
        float dot110 = glm::dot(grad110, d110);
        float dot001 = glm::dot(grad001, d001);
        float dot101 = glm::dot(grad101, d101);
        float dot011 = glm::dot(grad011, d011);
        float dot111 = glm::dot(grad111, d111);
        
        // 7. 三维线性插值
        // 在x方向插值
        float x00 = glm::mix(dot000, dot100, smooth.x);
        float x10 = glm::mix(dot010, dot110, smooth.x);
        float x01 = glm::mix(dot001, dot101, smooth.x);
        float x11 = glm::mix(dot011, dot111, smooth.x);
        
        // 在y方向插值
        float y0 = glm::mix(x00, x10, smooth.y);
        float y1 = glm::mix(x01, x11, smooth.y);
        
        // 在z方向插值
        float noise = glm::mix(y0, y1, smooth.z);
        
        // 8. 将结果从[-1,1]映射到[0,1]
        return noise * 0.5f + 0.5f;
    }
};

#endif