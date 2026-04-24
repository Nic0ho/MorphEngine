#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Morph
{

enum class CameraMovement { Forward, Backward, Left, Right, Up, Down };

class Camera
{
public:
    float MovementSpeed    = 5.0f;
    float MouseSensitivity = 0.1f;

    Camera() = default;
    Camera(glm::vec3 position, float fov, float aspect,
           float nearPlane = 0.1f, float farPlane = 1000.f);

    void ProcessKeyboard(CameraMovement direction, float dt);
    void ProcessMouseMovement(float dx, float dy, bool constrainPitch = true);

    void SetAspectRatio(float aspect);
    void SetFOV(float fov);
    void SetPosition(const glm::vec3& pos);

    glm::vec3 GetPosition()  const { return m_position; }
    glm::vec3 GetFront()     const { return m_front;    }
    float     GetFOV()       const { return m_fov;      }
    float     GetYaw()       const { return m_yaw;      }
    float     GetPitch()     const { return m_pitch;    }

    glm::mat4 GetViewMatrix()       const;
    glm::mat4 GetProjectionMatrix() const;

private:
    void UpdateVectors();

    glm::vec3 m_position = { 0.f, 0.f,  3.f };
    glm::vec3 m_front    = { 0.f, 0.f, -1.f };
    glm::vec3 m_up       = { 0.f, 1.f,  0.f };
    glm::vec3 m_right    = { 1.f, 0.f,  0.f };
    glm::vec3 m_worldUp  = { 0.f, 1.f,  0.f };

    float m_yaw   = -90.f;
    float m_pitch =   0.f;

    float m_fov    = 45.f;
    float m_aspect = 16.f / 9.f;
    float m_near   = 0.1f;
    float m_far    = 1000.f;
};

}