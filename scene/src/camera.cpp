#include "scene/camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace lucid::scene
{
    Camera::Camera(const CameraMode& CameraMode,
                   const glm::vec3& CameraPosition,
                   const glm::vec3& CameraUp,
                   const float& CameraYaw,
                   const float& CameraPitch,
                   const float& CameraSpeed,
                   const float& CameraSensitivity,
                   const float& CameraFOV)
    : Mode(CameraMode), Position(CameraPosition), UpVector(CameraUp),
      Yaw(CameraYaw), Pitch(CameraPitch), Speed(CameraSpeed), Sensitivity(CameraSensitivity),
      FOV(CameraFOV), WorldUpVector(CameraUp)
    {
        UpdateCameraVectors();
    }

    glm::mat4 Camera::GetViewMatrix() const 
    { 
        return glm::lookAt(Position, Position + FrontVector, UpVector); 
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {

        if (Mode == CameraMode::ORTHOGRAPHIC)
        {
            return glm::ortho(Left, Right, Bottom, Top, NearPlane, FarPlane);
        }
        return glm::perspective(glm::radians(FOV), AspectRatio,  0.1f, 100.0f);
    }

    void Camera::MoveForward(const float& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position += FrontVector * velocity;
    }

    void Camera::MoveBackward(const float& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position -= FrontVector * velocity;
    }

    void Camera::MoveRight(const float& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position += RightVector * velocity;
    }

    void Camera::MoveLeft(const float& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position -= RightVector * velocity;
    }

    void Camera::Move(const glm::vec3& DirectionVector, const float& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position += DirectionVector * velocity;
    }
    void Camera::AddRotation(float YawOffset, float PitchOffset, const bool& constrainPitch)
    {
        YawOffset *= Sensitivity;
        PitchOffset *= Sensitivity;

        Yaw += YawOffset;
        Pitch += PitchOffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        UpdateCameraVectors();
    }

    void Camera::UpdateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        newFront.y = sin(glm::radians(Pitch));
        newFront.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        FrontVector = glm::normalize(newFront);
        // also re-calculate the Right and Up vector
        RightVector = glm::normalize(glm::cross(FrontVector, WorldUpVector)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        UpVector = glm::normalize(glm::cross(RightVector, FrontVector));
    }
} // namespace lucid::scene
