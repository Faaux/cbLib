#pragma once
#include "cbInclude.h"
#include "imgui.h"
#include "GLM.h"
#include "cbKeys.h"

class Camera
{
public:
	//ToDo: Figure out AspectRatio!
	Camera(float fov, float near, float far, float aspectRatio, glm::vec3 pos, glm::vec3 lookAt) : _fov(fov),_near(near),_far(far),_position(pos)
	{
		_projectionMatrix = glm::perspective(
			glm::radians(_fov),
			aspectRatio,
			_near,
			_far
		);

		_forward = glm::normalize(lookAt - _position);
		_right = glm::cross(_forward, glm::vec3(0, 1, 0));
		_viewMatrix = glm::lookAt(_position, _position + _forward, glm::vec3(0, 1, 0));

	}

	const glm::mat4& GetProjectionMatrix() const
	{
		return _projectionMatrix;
	}

	const glm::mat4& GetViewMatrix() const
	{
		return _viewMatrix;
	}

	void SetPos(glm::vec3 pos)
	{
		if(_position != pos)
		{
			_position = pos;
			_viewMatrix = glm::lookAt(_position, _position + _forward, glm::vec3(0, 1, 0));
		}
	}

	void Update(float deltaTime, GameInput *input)
	{
		static bool isVisible = false;
		if (!input->OldKeyboardInput.Keys[cbKey_F1].IsDown && input->NewKeyboardInput.Keys[cbKey_F1].IsDown)
		{
			isVisible = !isVisible;
		}
		static float rotationSpeed = 4.f;
		static float speed = 2.f;
		if (isVisible && ImGui::Begin("Mouse Settings"))
		{
			ImGui::DragFloat("Camera Sensitivity", &rotationSpeed, 0.01f, 0.01f);
			ImGui::DragFloat("Movement Speed", &speed, 0.02f, 0.01f);
			ImGui::End();
		}

		// Update Pos by User Input
		glm::vec2 mouseDelta((int32)input->NewMouseInputState.X - (int32)input->OldMouseInputState.X, (int32)input->OldMouseInputState.Y - (int32)input->NewMouseInputState.Y);
		

		mouseDelta *= rotationSpeed / 1000.f;
		

		if(input->NewMouseInputState.MouseButtons[1])
		{
			glm::quat rotX = glm::angleAxis(-mouseDelta.x, glm::vec3(0.f, 1.f, 0.f));
			glm::quat rotY = glm::angleAxis(mouseDelta.y, _right);

			
			glm::vec3 newForward = (rotX * rotY) * _forward;// glm::vec4(_forward.x, _forward.y, _forward.z, 1.f);

			_forward = glm::normalize(glm::vec3(newForward.x, newForward.y, newForward.z));
			_right = glm::cross(_forward, glm::vec3(0, 1, 0));
			_viewMatrix = glm::lookAt(_position, _position + _forward, glm::vec3(0, 1, 0));
		}

		glm::vec3 dir(0);
		if(input->NewKeyboardInput.Keys[cbKey_W].IsDown)
		{			
			dir += _forward;
		}
		if (input->NewKeyboardInput.Keys[cbKey_S].IsDown)
		{
			dir -= _forward;
		}
		if(input->NewKeyboardInput.Keys[cbKey_D].IsDown)
		{			
			dir += _right;
		}
		if (input->NewKeyboardInput.Keys[cbKey_A].IsDown)
		{
			dir -= _right;
		}

		SetPos(_position + dir * speed * deltaTime);
	}
private:	
	float _fov, _near, _far;

	glm::vec3 _forward, _right;
	glm::vec3 _position;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
};
