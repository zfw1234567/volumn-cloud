#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f));
    
    glm::mat4 getViewMatrix();
    glm::mat4 getSkyboxViewMatrix();
    glm::mat4 getProjectionMatrix(float aspectRatio);
    
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);

    
    // 相机属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    
    // 欧拉角
    float Yaw;
    float Pitch;
    
    // 相机选项
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
    // 投影参数
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 5000.0f;
    
private:
    // 计算前向向量
    void updateCameraVectors();
};