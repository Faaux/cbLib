#include "cbInclude.h"
#include "cbCamera.h"
#include "cbGame.h"
#include "imgui.h"
#include "cbKeys.h"
#include "cbImgui.h"

Camera::Camera(float fov, float near, float far, float aspectRatio, glm::vec3 pos, glm::vec3 lookAt) : _fov(fov), _near(near), _far(far), _position(pos), _isTransitioning(false)
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
	TWEAKER(R1, "Camera Sensitivity", &rotationSpeed);
	TWEAKER(R1, "Movement Speed", &speed);
	TWEAKER(R3, "Cam Position", &_position.x);

	static bool wasTransitioning = false;
	if (!_isTransitioning)
	{
		wasTransitioning = false;
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
		if (PRESSED(input, cbKey_W))
		{
			dir += _forward;
		}
		if (PRESSED(input, cbKey_S))
		{
			dir -= _forward;
		}
		if (PRESSED(input, cbKey_D))
		{
			dir += _right;
		}
		if (PRESSED(input, cbKey_A))
		{
			dir -= _right;
		}
		if (PRESSED(input, cbKey_SPACEBAR))
		{
			dir += glm::vec3(0, 1, 0);
		}
		if (PRESSED(input, cbKey_SHIFT))
		{
			dir -= glm::vec3(0, 1, 0);;
		}

		SetPos(_position + dir * speed * deltaTime);
	}
	else
	{
		static float accum = 0;
		static glm::quat initalQuat;
		static glm::vec3 initalPos;

		accum += deltaTime;
		float lerp = accum / _transitionDelta;

		if (!wasTransitioning)
		{
			initalQuat = _currentRot;
			initalPos = _position;
		}


		if (accum <= _transitionDelta)
		{
			_position = glm::lerp(initalPos, _targetPos, lerp);
			_currentRot = glm::lerp(initalQuat, _targetRot, lerp);
		}
		else
		{
			_position = _targetPos;
			_currentRot = _targetRot;
			accum = 0;
			_isTransitioning = false;
		}

		CalculateViewMatrix();
		wasTransitioning = true;
	}
}

void Camera::TransitionTo(glm::quat rot, glm::vec3 pos, float delta)
{
	_isTransitioning = true;
	_targetPos = pos;
	_targetRot = rot;
	_transitionDelta = delta;
}

void Camera::CalculateViewMatrix()
{
	_viewMatrix = glm::inverse(glm::translate(_position) * glm::toMat4(_currentRot));

	_forward = -glm::normalize(glm::vec3(_viewMatrix[0][2], _viewMatrix[1][2], _viewMatrix[2][2]));
	_right = glm::normalize(glm::vec3(_viewMatrix[0][0], _viewMatrix[1][0], _viewMatrix[2][0]));
}
