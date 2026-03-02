#include "Camera.h"
#include <iostream>

Camera::Camera(glm::vec3 position) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
      WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
      Yaw(-90.0f), 
      Pitch(0.0f),
      MovementSpeed(2.5f),
      MouseSensitivity(0.1f),
      Zoom(45.0f) {
    Position = position;
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getSkyboxViewMatrix() {
    return glm::lookAt(glm::vec3(0.0f), Front, Up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;
    
    Yaw += xoffset;
    Pitch += yoffset;
    
    // 确保俯仰角不会超出限制
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }
    
    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    fov -= yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void Camera::updateCameraVectors() {
    // 计算新的前向向量
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    
    // 重新计算右向量和上向量
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}