#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    Camera(float aspectRatio, float zNear, float zFar, const glm::vec3& cameraPos, const glm::vec3& cameraFront,
           const glm::vec3& cameraUp, float speed, float fov, float mouseSensitivity) : 
        m_AspectRatio(aspectRatio),
        m_Znear(zNear),
        m_Zfar(zFar),
        m_CameraPos(cameraPos),
        m_CameraFront(cameraFront),
        m_CameraUp(cameraUp),
        m_CurrentDirection(glm::vec3(0.0f)),
        m_Speed(speed),
        m_BaseFov(fov),
        m_CurrentFov(fov),
        m_MouseSensitivity(mouseSensitivity),
        m_Zoom(1.0f),
        m_Yaw(-90.0f),
        m_Pitch(0.0f),
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
    void Zoom(int verticalScroll);

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

private:
    glm::mat4 m_Projection{};
    glm::vec3 m_CameraPos;
    glm::vec3 m_CameraFront;
    glm::vec3 m_CameraUp;
    glm::vec3 m_CurrentDirection;
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

template <CameraType T>
void Camera<T>::Zoom(int verticalScroll) {
    if constexpr (T == CameraType::PERSPECTIVE) {
        m_CurrentFov -= verticalScroll;

        // constrain fov
        if (m_CurrentFov < 1.0f)
            m_CurrentFov = 1.0f;
        if (m_CurrentFov > m_BaseFov)
            m_CurrentFov = m_BaseFov;

        m_Projection = glm::perspective(glm::radians(m_CurrentFov), m_AspectRatio, m_Znear, m_Zfar);
    } else {
        m_Zoom -= verticalScroll;
        m_Projection = glm::ortho(-m_AspectRatio*m_Zoom/2, m_AspectRatio*m_Zoom/2, -m_Zoom/2,
                m_Zoom/2, m_Znear, m_Zfar);
    }
}

