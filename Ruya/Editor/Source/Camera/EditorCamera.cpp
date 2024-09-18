#include "EditorCamera.h"
#include <Input/RInput.h>
#include <Core/Time.h>
#include <iostream>

REditor::EditorCamera::EditorCamera() : Camera()
{
    pitch = 0.0f;
    yaw = 0.0f;
}

REditor::EditorCamera::~EditorCamera()
{
}

void REditor::EditorCamera::Update()
{
    using namespace Ruya;

    if(RInput::GetMouseButton(RInput::MouseButton::RIGHT))
    {
        int windowWidth, windowHeight;

        RInput::SetCursorEnabled(false);

        const float cameraSpeed = 2.0f * Time::GetDeltaTime();

        if (RInput::GetKey(RInput::KeyCode::W))
            transform.position += cameraSpeed * transform.front;

        if (RInput::GetKey(RInput::KeyCode::S))
            transform.position -= cameraSpeed * transform.front;

        if (RInput::GetKey(RInput::KeyCode::A))
            transform.position -= math::Normalize(math::Cross(transform.front, transform.up)) * cameraSpeed;

        if (RInput::GetKey(RInput::KeyCode::D))
            transform.position += math::Normalize(math::Cross(transform.front, transform.up)) * cameraSpeed;

        if (RInput::GetKey(RInput::KeyCode::E))
            transform.position += transform.up * cameraSpeed;

        if (RInput::GetKey(RInput::KeyCode::Q))
            transform.position -= transform.up * cameraSpeed;

        float sensitivity = 0.1f;
        float deltaPosX;
        float deltaPosY;

        math::vec2 mouseDelta = RInput::GetMouseDelta();

        deltaPosX = mouseDelta.x * sensitivity;
        deltaPosY = mouseDelta.y * sensitivity;

        yaw += deltaPosX;
        pitch += deltaPosY;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        math::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        transform.front = math::Normalize(direction);
    }
    
    else
    {
        Ruya::RInput::firstMouse = true;
        RInput::SetCursorEnabled(true);
    }
}
