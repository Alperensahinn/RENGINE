#include "Time.h"

float Ruya::Time::deltaTime = 0.0f;
float Ruya::Time::lastFrameTime = 0.0f;

void Ruya::Time::UpdateTime()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrameTime;
    lastFrameTime = currentFrame;
}

float Ruya::Time::GetDeltaTime()
{
    return deltaTime;
}
