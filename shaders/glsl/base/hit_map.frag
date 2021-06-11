#version 420 core

uniform uint uActorId;

out uint oActorId;

void main() 
{
    oActorId = uActorId;
}