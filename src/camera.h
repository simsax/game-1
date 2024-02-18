#include "logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

enum class Direction {
    FORWARD,
    BACKWARDS,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

enum class CameraType {
    PERSPECTIVE,
    ORTHOGRAPHIC
};

template <CameraType T>
class Camera {
public:
    Camera(float screenWidth, float screenHeight, float zNear, float zFar, const glm::vec3& cameraPos,
            const glm::vec3& cameraFront, const glm::vec3& cameraUp, float speed, float fov,
            float mouseSensitivity, float zoom = 10.0f) :
        m_View(glm::mat4(1.0f)),
        m_ScreenWidth(screenWidth),
        m_ScreenHeight(screenHeight),
        m_AspectRatio(screenWidth / screenHeight),
        m_Znear(zNear),
        m_Zfar(zFar),
        m_CameraPos(cameraPos),
        m_CameraFront(cameraFront),
        m_CameraUp(cameraUp),
        m_CurrentDirection(glm::vec3(0.0f)),
        m_OrbitTarget(glm::vec3(0.0f)),
        m_Speed(speed),
        m_BaseFov(fov),
        m_CurrentFov(fov),
        m_MouseSensitivity(mouseSensitivity),
        m_Yaw(glm::degrees(atan2(m_CameraFront.z, m_CameraFront.x))),
        m_Pitch(glm::degrees(asin(m_CameraFront.y))),
        m_Zoom(zoom),
        m_Up(false),
        m_Down(false),
        m_Forward(false),
        m_Backwards(false),
        m_Left(false),
        m_Right(false)
    {
        if constexpr (T == CameraType::PERSPECTIVE) {
            m_Projection = glm::perspective(glm::radians(m_BaseFov), m_AspectRatio, m_Znear, m_Zfar);
        } else {
            m_Projection = glm::ortho(-m_AspectRatio*m_Zoom/2, m_AspectRatio*m_Zoom/2, -m_Zoom/2,
                    m_Zoom/2, m_Znear, m_Zfar);
        }
    }

    void Update(float deltaTime);
    void Move(Direction direction);
    void Stop(Direction direction);
    void Turn(int xoffset, int yoffset);
    void Orbit(int xoffset, int yoffset);
    void Pan(int xoffset, int yoffset);
    void Zoom(int verticalScroll);
    void SetPos(const glm::vec3& pos);

    inline const glm::vec3& GetPos() const {
        return m_CameraPos;
    }

    inline const glm::vec3& GetFront() const {
        return m_CameraFront;
    }

    inline const glm::vec3& GetUp() const {
        return m_CameraUp;
    }

    inline const glm::mat4& GetProjection() const {
        return m_Projection;
    }

    inline const glm::mat4& GetView() const {
        return m_View;
    }

    inline float GetOrthoWidth() const {
        return m_AspectRatio * m_Zoom;
    }

    inline float GetOrthoHeight() const {
        return m_Zoom;
    }

private:
    glm::mat4 m_Projection{};
    glm::mat4 m_View{};
    glm::vec3 m_CameraPos;
    glm::vec3 m_CameraFront;
    glm::vec3 m_CameraUp;
    glm::vec3 m_CurrentDirection;
    glm::vec3 m_OrbitTarget;
    float m_ScreenWidth;
    float m_ScreenHeight;
    float m_AspectRatio;
    float m_Znear;
    float m_Zfar;
    float m_Speed;
    float m_BaseFov;
    float m_CurrentFov;
    float m_MouseSensitivity;
    float m_Yaw;
    float m_Pitch;
    float m_Zoom;
    bool m_Up;
    bool m_Down;
    bool m_Forward;
    bool m_Backwards;
    bool m_Left;
    bool m_Right;
};

template <CameraType T>
void Camera<T>::Update(float deltaTime) {
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

    m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
}

template <CameraType T>
void Camera<T>::Move(Direction direction) {
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

template <CameraType T>
void Camera<T>::Stop(Direction direction) {
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


template <CameraType T>
void Camera<T>::Turn(int xoffset, int yoffset) {
    if constexpr (T == CameraType::PERSPECTIVE) {
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
    } else {
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

template <CameraType T>
void Camera<T>::Pan(int xoffset, int yoffset) {
    float xPanOffset;
    float yPanOffset;
    if constexpr (T == CameraType::PERSPECTIVE) {
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

template <CameraType T>
void Camera<T>::Zoom(int verticalScroll) {
    if constexpr (T == CameraType::PERSPECTIVE) {
        m_CurrentFov -= verticalScroll;

        // constrain fov
        if (m_CurrentFov < 1.0f)
            m_CurrentFov = 1.0f;

        m_Projection = glm::perspective(glm::radians(m_CurrentFov), m_AspectRatio, m_Znear, m_Zfar);
    } else {
        m_Zoom -= verticalScroll;
        m_Projection = glm::ortho(-m_AspectRatio*m_Zoom/2, m_AspectRatio*m_Zoom/2, -m_Zoom/2,
                m_Zoom/2, m_Znear, m_Zfar);
    }
}

template <CameraType T>
void Camera<T>::SetPos(const glm::vec3& pos) {
    m_CameraPos = pos;
}


template <CameraType T>
void Camera<T>::Orbit(int xoffset, int yoffset) {
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

