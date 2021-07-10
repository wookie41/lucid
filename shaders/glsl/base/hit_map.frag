#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"

flat in int InstanceID;
in vec4 oWorldPos;

layout (location = 0) out uint oActorId;
layout (location = 1) out float oDistanceToCamera;


void main() 
{
    oActorId = INSTANCE_DATA.ActorId;
    oDistanceToCamera = length(uViewPos - oWorldPos.xyz);
}