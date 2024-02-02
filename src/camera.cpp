#include "camera.h"

void Camera::Update(float deltaTime) {
    float cameraOffset = m_Speed * deltaTime; // distance traveled by the camera

    if (m_Forward)
        m_CameraPos += cameraOffset * glm::normalize(glm::vec3(m_CameraFront.x, 0, m_CameraFront.z));
    if (m_Backwards)
        m_CameraPos -= cameraOffset * glm::normalize(glm::vec3(m_CameraFront.x, 0, m_CameraFront.z));
    if (m_Left)
        m_CameraPos -= cameraOffset * glm::normalize(glm::cross(m_CameraFront, m_CameraUp));
    if (m_Right)
        m_CameraPos += cameraOffset * glm::normalize(glm::cross(m_CameraFront, m_CameraUp));
    if (m_Up)
        m_CameraPos.y += cameraOffset;
    if (m_Down)
        m_CameraPos.y -= cameraOffset;
}

void Camera::Move(Direction direction) {
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

void Camera::Stop(Direction direction) {
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


void Camera::Turn(int xoffset, int yoffset) {
    m_Yaw = glm::mod(m_Yaw + xoffset * m_MouseSensitivity, 360.0f);
    m_Pitch += yoffset * m_MouseSensitivity;

    // constrain m_Pitch to avoid weird camera movements
    static constexpr float maxpitch = 89.0f;
    if (m_Pitch > maxpitch)
        m_Pitch = maxpitch;
    if (m_Pitch < -maxpitch)
        m_Pitch = -maxpitch;

    // calculate camera direction
#if !ORTHO
    glm::vec3 direction;
    direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    direction.y = sin(glm::radians(m_Pitch));
    direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_CameraFront = glm::normalize(direction);
#endif
}

void Camera::Zoom(int verticalScroll) {
#if ORTHO
    //TODO: figure out
#else
    m_CurrentFov -= verticalScroll;

    // constrain fov
    if (m_CurrentFov < 1.0f)
        m_CurrentFov = 1.0f;
    if (m_CurrentFov > m_BaseFov)
        m_CurrentFov = m_BaseFov;

    m_Projection = 
#endif
}

glm::vec3 Camera::GetPos() const {
    return m_CameraPos;
}

glm::vec3 Camera::GetFront() const {
    return m_CameraFront;
}

glm::vec3 Camera::GetUp() const {
    return m_CameraUp;
}

glm::mat4 Camera::GetProjection() const {
    return m_Projection;
}
