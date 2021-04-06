#version 330 core

uniform uint uActorId;

out uint oActorId;

void main() 
{
    oActorId = uActorId;
}