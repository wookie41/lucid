#version 450 core

#include "batch_instance.glsl"

flat in int InstanceID;
out uint oActorId;

void main() 
{
    oActorId = INSTANCE_DATA.ActorId;
}