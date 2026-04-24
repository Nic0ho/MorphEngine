#include "Camera/MorphCamera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace Morph
{

Camera::Camera(glm::vec3 position, float fov, float aspect, float nearPlane, float farPlane)
    : m_position(position)
    , m_fov(fov)
    , m_aspect(aspect)
    , m_near(nearPlane)
    , m_far(farPlane)
{
    UpdateVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float dt)
{
    float velocity = MovementSpeed * dt;

    switch (direction)
    {
        case CameraMovement::Forward:  m_position += m_front   * velocity; break;
        case CameraMovement::Backward: m_position -= m_front   * velocity; break;
        case CameraMovement::Left:     m_position -= m_right   * velocity; break;
        case CameraMovement::Right:    m_position += m_right   * velocity; break;
        case CameraMovement::Up:       m_position += m_worldUp * velocity; break;
        case CameraMovement::Down:     m_position -= m_worldUp * velocity; break;
    }
}

void Camera::ProcessMouseMovement(float dx, float dy, bool constrainPitch)
{
    m_yaw   += dx * MouseSensitivity;
    m_pitch += dy * MouseSensitivity;

    if (constrainPitch)
        m_pitch = std::clamp(m_pitch, -89.f, 89.f);

    UpdateVectors();
}

void Camera::SetAspectRatio(float aspect) { m_aspect = aspect; }
void Camera::SetFOV(float fov)            { m_fov    = fov;    }
void Camera::SetPosition(const glm::vec3& pos) { m_position = pos; }

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    glm::mat4 proj = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    proj[1][1] *= -1.f;
    return proj;
}

void Camera::UpdateVectors()
{
    glm::vec3 front;
    front.x = glm::cos(glm::radians(m_yaw))   * glm::cos(glm::radians(m_pitch));
    front.y = glm::sin(glm::radians(m_pitch));
    front.z = glm::sin(glm::radians(m_yaw))   * glm::cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

}