#pragma once

#include "glm/glm.hpp"
#include "common/types.hpp"
#include "misc/math.hpp"

namespace lucid::scene
{
    enum class ECameraMode : u8
    {
        ORTHOGRAPHIC,
        PERSPECTIVE
    };

    class CCamera
    {
      public:
        CCamera(const ECameraMode& CameraMode,
                const glm::vec3&   CameraPosition    = { 0.0, 0.0, 0.0 },
                const glm::vec3&   CameraUp          = { 0.0, 1.0, 0.0 },
                const real&        CameraYaw         = 0,
                const real&        CameraPitch       = 0.0,
                const real&        CameraSpeed       = 7.5,
                const real&        CameraSensitivity = 0.5,
                const real&        CameraZoom        = 45.0);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;
        glm::mat4 GetOrthoProjectionMatrix() const;

        void MoveForward(const real& DeltaTime);
        void MoveBackward(const real& DeltaTime);
        void MoveLeft(const real& DeltaTime);
        void MoveRight(const real& DeltaTime);
        void Move(const glm::vec3& DirectionVector, const float& DeltaTime);

        // calculates the front vector from the Camera's (updated) Euler Angles
        // called automatically when you use AddRotation(), but has to be called manually when
        // any of the camera's properties are updated by hand
        void UpdateCameraVectors();
        void UpdateFrustumAABB();

        void AddRotation(real YawOffset, real PitchOffset, const bool& constrainPitch = true);

        glm::vec3 GetMouseRayInViewSpace(const glm::vec2& InMousePosNDC, const float InT = 1) const;
        glm::vec3 GetMouseRayInWorldSpace(const glm::vec2& InMousePosNDC, const float InT = 1) const;

        void Tick(const float& DeltaTime);
        void MoveToOverTime(const glm::vec3& InLocation, const glm::vec3& InDirection, const float& InDuration);

        inline void SetAspectRatio(const float& InAspectRatio) { AspectRatio = InAspectRatio; }
        inline void SetYaw(const float& InYaw) { Yaw = InYaw; }
        inline void SetPosition(const glm::vec3& InPosition) { Position = InPosition; }
        inline void SetDirection(const glm::vec3& InDirection) { FrontVector = InDirection;}
        inline void SetNearPlane(const float& InNearPlane) { NearPlane = InNearPlane; }
        inline void SetFarPlane(const float& InFarPlane) { FarPlane = InFarPlane; }
        inline void SetCameraUp(const glm::vec3& InCameraUp) { UpVector = InCameraUp; }

        inline glm::vec3 GetPosition() const { return Position; }
        inline glm::vec3 GetDirection() const { return FrontVector; }
        inline float     GetNearPlane() const { return NearPlane; }
        inline float     GetFarPlane() const { return FarPlane; }
        inline float     GetYaw() const { return Yaw; }
        inline glm::vec3 GetCameraUp() const { return UpVector; }

        const math::FAABB& GetFrustumAABB() const { return FrustumAABB; }

      protected:
        math::FAABB FrustumAABB;

        real NearPlane = 0.1;
        real FarPlane  = 10000.0;

        real Left = 0, Right = 0;
        real Bottom = 0, Top = 0;

        ECameraMode Mode;
        real        AspectRatio = 0;

        glm::vec3 Position;
        glm::vec3 FrontVector;
        glm::vec3 UpVector;
        glm::vec3 RightVector;
        glm::vec3 WorldUpVector;

        real Yaw;
        real Pitch;

        real Speed;
        real Sensitivity;
        real FOV;

        float MoveDuration    = 0;
        float CurrentMoveTime = 0;
        bool  bMoveRequested  = false;

        glm::vec3 StartingPosition{ 0, 0, 0 };
        glm::vec3 StartingDirection{ 0, 0, 0 };

        glm::vec3 DesiredPosition{ 0, 0, 0 };
        glm::vec3 DesiredDirection{ 0, 0, 0 };
    };
} // namespace lucid::scene