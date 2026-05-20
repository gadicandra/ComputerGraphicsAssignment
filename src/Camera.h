#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch;
    float MovementSpeed = 2.5f;
    float MouseSensitivity = 0.1f;
    float Zoom = 45.0f;

    Camera(glm::vec3 pos = glm::vec3(0,0,3),
           glm::vec3 up  = glm::vec3(0,1,0),
           float yaw = -90.0f, float pitch = 0.0f);

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }
    void ProcessKeyboard(CameraMovement dir, float dt);
    void ProcessMouseMovement(float xoff, float yoff, bool constrainPitch = true);
    void ProcessMouseScroll(float yoff);

private:
    void updateVectors();
};