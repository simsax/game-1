#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define ORTHO 0

enum class Direction {
    FORWARD,
    BACKWARDS,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// TODO: for ortographic camera, just overload the constructor maybe
// TODO: figure out zoom (just pass different parameter to construct the projection for each constructor)
class Camera {
public:
    Camera(const glm::mat4& projection, const glm::vec3& cameraPos, const glm::vec3& cameraFront,
           const glm::vec3& cameraUp, float speed, float fov, float mouseSensitivity) : 
        m_Projection(projection),
        m_CameraPos(cameraPos),
        m_CameraFront(cameraFront),
        m_CameraUp(cameraUp),
        m_CurrentDirection(glm::vec3(0.0f)),
        m_Speed(speed),
        m_BaseFov(fov),
        m_CurrentFov(fov),
        m_MouseSensitivity(mouseSensitivity),
        m_Yaw(-90.0f),
        m_Pitch(0.0f),
        m_Up(false),
        m_Down(false),
        m_Forward(false),
        m_Backwards(false),
        m_Left(false),
        m_Right(false)
    { }

    void Update(float deltaTime);
    void Move(Direction direction);
    void Stop(Direction direction);
    void Turn(int xoffset, int yoffset);
    void Zoom(int verticalScroll);
    glm::vec3 GetPos() const;
    glm::vec3 GetFront() const;
    glm::vec3 GetUp() const;
    glm::mat4 GetProjection() const;

private:
    glm::mat4 m_Projection;
    glm::vec3 m_CameraPos;
    glm::vec3 m_CameraFront;
    glm::vec3 m_CameraUp;
    glm::vec3 m_CurrentDirection;
    float m_Speed;
    float m_BaseFov;
    float m_CurrentFov;
    float m_MouseSensitivity;
    float m_Yaw;
    float m_Pitch;
    bool m_Up;
    bool m_Down;
    bool m_Forward;
    bool m_Backwards;
    bool m_Left;
    bool m_Right;
};
