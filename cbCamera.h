#pragma once
#include "cbInclude.h"
#include "imgui.h"
#include "GLM.h"
#include "cbKeys.h"

inline cbInternal glm::quat RotBetweenVectors(glm::vec3 dest, glm::vec3 start)
{
	// Based on Stan Melax's article in Game Programming Gems
	glm::quat q;
	// Copy, since cannot modify local
	glm::vec3 v0 = start;
	glm::vec3 v1 = dest;

	v0 = normalize(v0);
	v1 = normalize(v1);

	float d = dot(v0,v1);
	// If dot == 1, vectors are the same
	if (d >= 1.0f)
	{
		return glm::quat();
	}
	if (d < (1e-6f - 1.0f))
	{
		
			// Generate an axis
			glm::vec3 axis = cross(glm::vec3(1,0,0),start);
			if (length(axis) < 1e-6f) // pick another if colinear
				axis = cross(glm::vec3(0, 1, 0), start);
			axis = normalize(axis);
			q = glm::quat(Pi, axis);
		
	}
	else
	{
		float s = sqrt((1 + d) * 2);
		float invs = 1 / s;

		glm::vec3 c = cross(v0,v1);

		q.x = c.x * invs;
		q.y = c.y * invs;
		q.z = c.z * invs;
		q.w = s * 0.5f;
		q = normalize(q);
	}
	return q;
}

class Camera
{
public:
	//ToDo: Figure out AspectRatio!
	Camera(float fov, float near, float far, float aspectRatio, glm::vec3 pos, glm::vec3 lookAt) : _fov(fov), _near(near), _far(far), _position(pos)
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

		_currentRot = RotBetweenVectors(_forward, glm::vec3(0, 1, 0));
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
		if (_position != pos)
		{
			_position = pos;
			_viewMatrix = glm::lookAt(_position, _position + _forward, glm::vec3(0, 1, 0));
		}
	}

	void Update(float deltaTime, GameInput *input)
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
			
			if (isVisible && ImGui::Begin("Mouse Settings"))
			{
				ImGui::DragFloat("Camera Sensitivity", &rotationSpeed, 0.01f, 0.01f);
				ImGui::DragFloat("Movement Speed", &speed, 0.02f, 0.01f);
				ImGui::End();
			}
		}

		// Update Pos by User Input
		glm::vec2 mouseDelta((int32)input->NewMouseInputState.X - (int32)input->OldMouseInputState.X, (int32)input->OldMouseInputState.Y - (int32)input->NewMouseInputState.Y);


		mouseDelta *= rotationSpeed / 1000.f;


		if (input->NewMouseInputState.MouseButtons[1])
		{
			glm::quat rotX = glm::angleAxis(-mouseDelta.x, glm::vec3(0.f, 1.f, 0.f));
			glm::quat rotY = glm::angleAxis(mouseDelta.y, _right);

			_currentRot = rotX * rotY * _currentRot;

			_forward = glm::normalize(_currentRot * glm::vec3(0,1,0));
			_right = glm::cross(_forward, glm::vec3(0, 1, 0));
			_viewMatrix = glm::lookAt(_position, _position + _forward, glm::vec3(0, 1, 0));
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

		SetPos(_position + dir * speed * deltaTime);
	}
private:
	float _fov, _near, _far;

	glm::quat _currentRot;
	glm::vec3 _forward, _right;
	glm::vec3 _position;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
};
