#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 up, float yaw, float pitch)
    : Position(pos), WorldUp(up), Yaw(yaw), Pitch(pitch) {
    updateVectors();
}

void Camera::ProcessKeyboard(CameraMovement dir, float dt) {
    float v = MovementSpeed * dt;
    if (dir == FORWARD)  Position += Front * v;
    if (dir == BACKWARD) Position -= Front * v;
    if (dir == LEFT)     Position -= Right * v;
    if (dir == RIGHT)    Position += Right * v;
}

void Camera::ProcessMouseMovement(float xoff, float yoff, bool constrainPitch) {
    Yaw   += xoff * MouseSensitivity;
    Pitch += yoff * MouseSensitivity;
    if (constrainPitch) {
        if (Pitch >  89.0f) Pitch =  89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }
    updateVectors();
}

void Camera::ProcessMouseScroll(float yoff) {
    Zoom -= yoff;
    if (Zoom < 1.0f)  Zoom = 1.0f;
    if (Zoom > 90.0f) Zoom = 90.0f;
}

void Camera::updateVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    f.y = sin(glm::radians(Pitch));
    f.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(f);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}