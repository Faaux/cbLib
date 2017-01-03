#pragma once
#include "cbInclude.h"
#include "cbPlatform.h"
#include "GLM.h"

static const glm::vec3 VEC_X = glm::vec3(1, 0, 0);
static const glm::vec3 VEC_Y = glm::vec3(0, 1, 0);
static const glm::vec3 VEC_Z = glm::vec3(0, 0, 1);


class Camera
{
public:
	//ToDo: Figure out AspectRatio!
	Camera(float fov, float near, float far, float aspectRatio, glm::vec3 pos, glm::vec3 lookAt);

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
			CalculateViewMatrix();
		}
	}

	void Update(float deltaTime, GameInput *input);

	void TransitionTo(glm::quat rot, glm::vec3 pos, float delta);

public:

	void CalculateViewMatrix();

	bool _isTransitioning;
	float _fov, _near, _far;
	float _transitionDelta;

	glm::quat _currentRot;
	glm::quat _targetRot;
	glm::vec3 _forward, _right;
	glm::vec3 _position;
	glm::vec3 _targetPos;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
};
