#include "camera.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <SDL2/SDL.h>

void Camera::Update(float deltaTime) {
    float cameraDistance = m_Speed * deltaTime;

    if (m_Forward)
        m_CameraPos += cameraDistance * glm::normalize(glm::vec3(m_CameraFront.x, 0, m_CameraFront.z));
    if (m_Backwards)
        m_CameraPos -= cameraDistance * glm::normalize(glm::vec3(m_CameraFront.x, 0, m_CameraFront.z));
    if (m_Left)
        m_CameraPos -= cameraDistance * glm::normalize(glm::cross(m_CameraFront, m_CameraUp));
    if (m_Right)
        m_CameraPos += cameraDistance * glm::normalize(glm::cross(m_CameraFront, m_CameraUp));
    if (m_Up)
        m_CameraPos.y += cameraDistance;
    if (m_Down)
        m_CameraPos.y -= cameraDistance;

    m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
}

void Camera::Move(Direction direction) {
    if (m_Mode == CameraMode::FLY) {
        switch (direction) {
            case Direction::FORWARD:
                m_Forward = true;
                break;
            case Direction::BACKWARDS:
                m_Backwards = true;
                break;
            case Direction::LEFT:
                m_Left = true;
                break;
            case Direction::RIGHT:
                m_Right = true;
                break;
            case Direction::UP:
                m_Up = true;
                break;
            case Direction::DOWN:
                m_Down = true;
                break;
        }
    }
}

void Camera::Stop(Direction direction) {
    if (m_Mode == CameraMode::FLY) {
        switch (direction) {
            case Direction::FORWARD:
                m_Forward = false;
                break;
            case Direction::BACKWARDS:
                m_Backwards = false;
                break;
            case Direction::LEFT:
                m_Left = false;
                break;
            case Direction::RIGHT:
                m_Right = false;
                break;
            case Direction::UP:
                m_Up = false;
                break;
            case Direction::DOWN:
                m_Down = false;
                break;
        }
    }
}


void Camera::Turn(int xoffset, int yoffset) {
    if (m_Type == CameraType::PERSPECTIVE) {
        m_Yaw = glm::mod(m_Yaw + xoffset * m_MouseSensitivity, 360.0f);
        m_Pitch += yoffset * m_MouseSensitivity;

        // constrain m_Pitch to avoid weird camera movements
        static constexpr float maxpitch = 89.0f;
        if (m_Pitch > maxpitch)
            m_Pitch = maxpitch;
        if (m_Pitch < -maxpitch)
            m_Pitch = -maxpitch;

        // calculate camera direction
        glm::vec3 direction;
        direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        direction.y = sin(glm::radians(m_Pitch));
        direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_CameraFront = glm::normalize(direction);
    }
}

void Camera::Pan(int xoffset, int yoffset) {
    float xPanOffset;
    float yPanOffset;
    if (m_Type == CameraType::PERSPECTIVE) {
        static constexpr float speed = 20.0f;
        xPanOffset = - xoffset / m_ScreenWidth * speed;
        yPanOffset = - yoffset / m_ScreenHeight * speed;
    } else {
        xPanOffset = - xoffset * GetOrthoWidth() / m_ScreenWidth;
        yPanOffset = - yoffset * GetOrthoHeight() / m_ScreenHeight;
    }

    glm::vec3 right = glm::normalize(glm::cross(m_CameraFront, m_CameraUp));
    glm::vec3 up = glm::normalize(glm::cross(right, m_CameraFront));

    glm::vec3 panOffset = right * xPanOffset + up * yPanOffset;
    m_CameraPos += panOffset;
    m_OrbitTarget += panOffset;
}

void Camera::Zoom(int verticalScroll) {
    if (m_Type == CameraType::PERSPECTIVE) {
        /* static constexpr float zoomSpeed = 1.0f; */
        /* float distance = verticalScroll * zoomSpeed; */

        /* m_CameraPos += m_CameraFront * distance; */
        m_CurrentFov -= verticalScroll;

        // constrain fov
        if (m_CurrentFov < 1.0f)
            m_CurrentFov = 1.0f;
        /* if (m_CurrentFov > m_BaseFov) */
        /*     m_CurrentFov = m_BaseFov; */

        m_Perspective = glm::perspective(glm::radians(m_CurrentFov), m_AspectRatio, m_Znear, m_Zfar);
    } else {
        m_Zoom -= verticalScroll;
        m_Orthographic = glm::ortho(-m_AspectRatio*m_Zoom/2, m_AspectRatio*m_Zoom/2, -m_Zoom/2,
                m_Zoom/2, m_Znear, m_Zfar);
    }
}

void Camera::SetPos(const glm::vec3& pos) {
    m_CameraPos = pos;
}

void Camera::Orbit(int xoffset, int yoffset) {
    /*
       this is fine, blender style orbit camera probably requires a completely different 
       approach, since these calculation always use the positive y direction as the up vector
   */
    static constexpr float orbitSpeed = 0.25f;

    // If the camera is looking straight up and an upward rotation is attempted
    if (glm::dot(m_CameraUp, m_CameraFront) > 0.99f && yoffset > 0) {
        return;
    }
    // If the camera is looking straight down and a downward rotation is attempted
    if (glm::dot(m_CameraUp, m_CameraFront) < -0.99f && yoffset < 0) {
        return;
    }

    float xOrbitOffset = -xoffset;
    float yOrbitOffset = -yoffset;

    glm::vec3 hozAxis = glm::normalize(glm::cross(m_CameraFront, m_CameraUp));

    glm::mat4 transTarget = glm::translate(glm::mat4(1.0f), -m_OrbitTarget);
    glm::mat4 transTargetBack = glm::translate(glm::mat4(1.0f), m_OrbitTarget);

    glm::quat rotation = glm::angleAxis(glm::radians(xOrbitOffset) * orbitSpeed, m_CameraUp) *
        glm::angleAxis(glm::radians(yOrbitOffset) * orbitSpeed, -hozAxis);

    m_CameraPos = transTargetBack * toMat4(rotation) * transTarget * glm::vec4(
            m_CameraPos.x, m_CameraPos.y, m_CameraPos.z, 1);

    m_CameraFront = glm::normalize(m_OrbitTarget - m_CameraPos);
}

void Camera::FlipMode() {
    if (m_Mode == CameraMode::ORBIT) {
        m_Mode = CameraMode::FLY;
        SDL_SetRelativeMouseMode(SDL_TRUE);
    } else {
        m_Mode = CameraMode::ORBIT;
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void Camera::FlipType() {
    m_Type = m_Type == CameraType::PERSPECTIVE ? CameraType::ORTHOGRAPHIC : CameraType::PERSPECTIVE;
}
