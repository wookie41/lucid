#pragma once

#include "glm/glm.hpp"

namespace lucid::scene
{
    enum class CameraMode : uint8_t
    {
        ORTHOGRAPHIC,
        PERSPECTIVE
    };

    class Camera
    {
      public:
        Camera(const CameraMode& CameraMode,
               const glm::vec3& CameraPosition = { 0.0, 0.0, 1.0 },
               const glm::vec3& CameraUp = { 0.0, 1.0, 0.0 },
               const float& CameraYaw = -90.0,
               const float& CameraPitch = 0.0,
               const float& CameraSpeed = 2.5,
               const float& CameraSensitivity = 0.5,
               const float& CameraZoom = 45.0);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;

        void MoveForward(const float& DeltaTime);
        void MoveBackward(const float& DeltaTime);
        void MoveLeft(const float& DeltaTime);
        void MoveRight(const float& DeltaTime);
        void Move(const glm::vec3& DirectionVector, const float& DeltaTime);

        void AddRotation(float YawOffset, float PitchOffset, const bool& constrainPitch = true);

        float NearPlane = 0.1;
        float FarPlane = 100.0;

        float Left = 0, Right = 0;
        float Bottom = 0, Top = 0;

        CameraMode Mode;
        float AspectRatio = 0;

        glm::vec3 Position;
        glm::vec3 FrontVector;
        glm::vec3 UpVector;
        glm::vec3 RightVector;
        glm::vec3 WorldUpVector;

        float Yaw;
        float Pitch;

        float Speed;
        float Sensitivity;
        float FOV;

      private:
        // calculates the front vector from the Camera's (updated) Euler Angles
        void updateCameraVectors();
    };
} // namespace lucid::scene