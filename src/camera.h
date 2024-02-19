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

enum class CameraMode {
    FLY,
    ORBIT
};

class Camera {
public:
    Camera(float screenWidth, float screenHeight, float zNear, float zFar, const glm::vec3& cameraPos,
            const glm::vec3& cameraFront, const glm::vec3& cameraUp, float speed, float fov,
            float mouseSensitivity, float zoom = 10.0f) :
        m_Perspective(glm::perspective(glm::radians(fov), screenWidth / screenHeight, zNear, zFar)),
        m_Orthographic(glm::ortho(
                    -screenWidth / screenHeight * zoom/2, screenWidth / screenHeight * zoom/2,
                    -zoom/2, zoom/2, zNear, zFar)),
        m_View(glm::mat4(1.0f)),
        m_CameraPos(cameraPos),
        m_CameraFront(cameraFront),
        m_CameraUp(cameraUp),
        m_CurrentDirection(glm::vec3(0.0f)),
        m_OrbitTarget(glm::vec3(0.0f)),
        m_ScreenWidth(screenWidth),
        m_ScreenHeight(screenHeight),
        m_AspectRatio(screenWidth / screenHeight),
        m_Znear(zNear),
        m_Zfar(zFar),
        m_Speed(speed),
        m_BaseFov(fov),
        m_CurrentFov(fov),
        m_MouseSensitivity(mouseSensitivity),
        m_Yaw(glm::degrees(atan2(m_CameraFront.z, m_CameraFront.x))),
        m_Pitch(glm::degrees(asin(m_CameraFront.y))),
        m_Zoom(zoom),
        m_Type(CameraType::PERSPECTIVE),
        m_Mode(CameraMode::ORBIT),
        m_Up(false),
        m_Down(false),
        m_Forward(false),
        m_Backwards(false),
        m_Left(false),
        m_Right(false)
    {
    }

    void Update(float deltaTime);
    void Move(Direction direction);
    void Stop(Direction direction);
    void Turn(int xoffset, int yoffset);
    void Orbit(int xoffset, int yoffset);
    void Pan(int xoffset, int yoffset);
    void Zoom(int verticalScroll);
    void SetPos(const glm::vec3& pos);
    void FlipMode();
    void FlipType();

    inline const glm::vec3& GetPos() const {
        return m_CameraPos;
    }

    inline const glm::vec3& GetFront() const {
        return m_CameraFront;
    }

    inline const glm::vec3& GetUp() const {
        return m_CameraUp;
    }

    inline const glm::mat4& GetPerspectiveProjection() const {
        return m_Perspective;
    }

    inline const glm::mat4& GetOrthographicProjection() const {
        return m_Orthographic;
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

    inline CameraMode GetMode() const {
        return m_Mode;
    }

    inline CameraType GetType() const {
        return m_Type;
    }

private:
    glm::mat4 m_Perspective;
    glm::mat4 m_Orthographic;
    glm::mat4 m_View;
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
    CameraType m_Type;
    CameraMode m_Mode;
    bool m_Up;
    bool m_Down;
    bool m_Forward;
    bool m_Backwards;
    bool m_Left;
    bool m_Right;
};

