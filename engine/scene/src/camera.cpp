#include "scene/camera.hpp"

#include <glm/ext/quaternion_common.hpp>
#include <misc/math.hpp>

#include "glm/gtc/matrix_transform.hpp"

namespace lucid::scene
{
    CCamera::CCamera(const ECameraMode& CameraMode,
                     const glm::vec3&   CameraPosition,
                     const glm::vec3&   CameraUp,
                     const real&        CameraYaw,
                     const real&        CameraPitch,
                     const real&        CameraSpeed,
                     const real&        CameraSensitivity,
                     const real&        CameraFOV)
    : Mode(CameraMode), Position(CameraPosition), UpVector(CameraUp), Yaw(CameraYaw), Pitch(CameraPitch), Speed(CameraSpeed),
      Sensitivity(CameraSensitivity), FOV(CameraFOV), WorldUpVector(CameraUp)
    {
        UpdateCameraVectors();
    }

    glm::mat4 CCamera::GetViewMatrix() const { return glm::lookAt(Position, Position + FrontVector, UpVector); }

    glm::mat4 CCamera::GetProjectionMatrix() const
    {
        if (Mode == ECameraMode::ORTHOGRAPHIC)
        {
            return glm::ortho(Left, Right, Bottom, Top, NearPlane, FarPlane);
        }
        return glm::perspective(glm::radians(FOV), AspectRatio, NearPlane, FarPlane);
    }

    glm::mat4 CCamera::GetOrthoProjectionMatrix() const { return glm::ortho(Left, Right, Bottom, Top, NearPlane, FarPlane); }

    void CCamera::MoveForward(const real& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position += FrontVector * velocity;
    }

    void CCamera::MoveBackward(const real& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position -= FrontVector * velocity;
    }

    void CCamera::MoveRight(const real& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position += RightVector * velocity;
    }

    void CCamera::MoveLeft(const real& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position -= RightVector * velocity;
    }

    void CCamera::Move(const glm::vec3& DirectionVector, const real& DeltaTime)
    {
        float velocity = Speed * DeltaTime;
        Position += DirectionVector * velocity;
    }
    void CCamera::AddRotation(real YawOffset, real PitchOffset, const bool& constrainPitch)
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

    void CCamera::UpdateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 newFront;
        newFront.x  = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        newFront.y  = sin(glm::radians(Pitch));
        newFront.z  = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        FrontVector = glm::normalize(newFront);
        // also re-calculate the Right and Up vector
        RightVector = glm::normalize(glm::cross(FrontVector, WorldUpVector)); // normalize the vectors, because their length gets closer to 0 the more
                                                                              // you look up or down which results in slower movement.
        UpVector = glm::normalize(glm::cross(RightVector, FrontVector));
    }

    glm::vec3 CCamera::GetMouseRayInViewSpace(const glm::vec2& InMousePosNDC, const float InT) const
    {
        const glm::vec4 MouseRayClip{ InMousePosNDC, -1, 1 };
        const glm::vec4 MouseRayView = glm::inverse(GetProjectionMatrix()) * MouseRayClip;
        return glm::normalize(glm::vec3(MouseRayView)) * InT;
    }

    glm::vec3 CCamera::GetMouseRayInWorldSpace(const glm::vec2& InMousePosNDC, const float InT) const
    {
        const glm::vec4 MouseRayView = glm::vec4(GetMouseRayInViewSpace(InMousePosNDC, InT), 1);
        return glm::vec3(glm::inverse(GetViewMatrix()) * MouseRayView);
    }

    void CCamera::FocusOnLocation(const glm::vec3& InLocation, const float& InCameraZTranslation, const float& InCameraYTranslation)
    {
        Position = InLocation + glm::vec3{ 0, InCameraZTranslation, InCameraYTranslation };
        Yaw      = -90.f;
        Pitch    = -45;
        UpdateCameraVectors();
    }

    void CCamera::Update(const float& DeltaTime)
    {
        if (MoveDuration > 0)
        {
            if (glm::length(Position - DesiredPos) < 0.01 || glm::abs(DesiredYaw - Yaw) < 0.1 || glm::abs(DesiredPitch - Pitch) < 0.1)
            {
                MoveDuration = 0;
            }
            else if (CurrenteMoveTime > MoveDuration)
            {
                MoveDuration = 0;
                Position     = DesiredPos;
                Yaw          = DesiredYaw;
                Pitch        = DesiredPitch;
            }
            else
            {
                const float T = CurrenteMoveTime / MoveDuration;
                Position      = math::Lerp(Position, DesiredPos, T);
                Yaw           = math::Lerp(Yaw, DesiredYaw, T);
                Pitch         = math::Lerp(Pitch, DesiredPitch, T);
            }

            UpdateCameraVectors();
            CurrenteMoveTime += DeltaTime;
        }
    }

    void CCamera::MoveToOverTime(const glm::vec3& InLocation, const float& InYaw, const float& InPitch, const float& InDuration)
    {
        CurrenteMoveTime = 0;
        MoveDuration     = InDuration;
        DesiredPos       = InLocation;
        DesiredYaw       = InYaw;
        DesiredPitch     = InPitch;
    }

} // namespace lucid::scene
