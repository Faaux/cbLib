#include "cbInclude.h"
#include "cbCamera.h"
#include "cbGame.h"
#include "imgui.h"
#include "cbKeys.h"

Camera::Camera(float fov, float near, float far, float aspectRatio, glm::vec3 pos, glm::vec3 lookAt) : _fov(fov), _near(near), _far(far), _position(pos)
{
	_projectionMatrix = glm::perspective(
		glm::radians(_fov),
		aspectRatio,
		_near,
		_far
	);
	
	_forward = glm::normalize(lookAt - pos);
	_currentRot = glm::rotation(VEC_Z, -_forward);

	CalculateViewMatrix();
}

void Camera::Update(float deltaTime, GameInput* input)
{
	static float rotationSpeed = 4.f;
	static float speed = 2.f;

	// Imgui Debug Interface
	{
		static bool isVisible = false;
		if (!input->OldKeyboardInput.Keys[cbKey_F1].IsDown && input->NewKeyboardInput.Keys[cbKey_F1].IsDown)
		{
			isVisible = !isVisible;
		}

		if (isVisible)
		{
			if(!ImGui::Begin("Mouse Settings"))
			{
				ImGui::End();
				return;
			}
			ImGui::DragFloat("Camera Sensitivity", &rotationSpeed, 0.01f, 0.01f);
			ImGui::DragFloat("Movement Speed", &speed, 0.02f, 0.01f);

			ImGui::DragFloat3("Cam Pos", &_position.x);
			ImGui::End();
		}
	}

	if (input->NewMouseInputState.MouseButtons[1])
	{
		// Update Pos by User Input
		glm::vec2 mouseDelta((int32)input->NewMouseInputState.X - (int32)input->OldMouseInputState.X, (int32)input->OldMouseInputState.Y - (int32)input->NewMouseInputState.Y);
		mouseDelta *= rotationSpeed / 1000.f;

		//glm::quat keyQuat(glm::vec3(mouseDelta.y, -mouseDelta.x, 0));
		glm::quat rotX = glm::angleAxis(-mouseDelta.x, glm::vec3(0.f, 1.f, 0.f));
		glm::quat rotY = glm::angleAxis(mouseDelta.y, _right);

		_currentRot = glm::normalize(rotX * rotY * _currentRot);

		CalculateViewMatrix();
	}

	glm::vec3 dir(0);
	if (input->NewKeyboardInput.Keys[cbKey_W].IsDown)
	{
		dir += _forward;
	}
	if (input->NewKeyboardInput.Keys[cbKey_S].IsDown)
	{
		dir -= _forward;
	}
	if (input->NewKeyboardInput.Keys[cbKey_D].IsDown)
	{
		dir += _right;
	}
	if (input->NewKeyboardInput.Keys[cbKey_A].IsDown)
	{
		dir -= _right;
	}
	if (input->NewKeyboardInput.Keys[cbKey_SPACEBAR].IsDown)
	{
		dir += glm::vec3(0,1,0);
	}
	if (input->NewKeyboardInput.Keys[cbKey_SHIFT].IsDown)
	{
		dir -= glm::vec3(0, 1, 0);;
	}

	SetPos(_position + dir * speed * deltaTime);
}

void Camera::CalculateViewMatrix()
{
	_viewMatrix = glm::inverse(glm::translate(_position) * glm::toMat4(_currentRot));

	_forward = -glm::normalize(glm::vec3(_viewMatrix[0][2], _viewMatrix[1][2], _viewMatrix[2][2]));
	_right = glm::normalize(glm::vec3(_viewMatrix[0][0], _viewMatrix[1][0], _viewMatrix[2][0]));
}
