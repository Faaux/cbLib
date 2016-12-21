#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 uvCoords;
out vec3 normalCameraSpace;
out vec3 toEye;
out vec3 lightDirection;
out float distance;

uniform  vec3 lightPos;//		=	vec3(0,1,3);

void main()
{
	uvCoords = uv;

	mat4 mv = viewMatrix * modelMatrix;
	mat4 mvp = projectionMatrix * mv;
	gl_Position = mvp * vec4(position, 1);

	vec3 posCamSpace = (mv * vec4(position, 1)).xyz;
	vec3 lightCamSpace = (viewMatrix * vec4(lightPos,1)).xyz;
	vec3 normalCamSpace = (inverse(transpose(mv)) * vec4(normal, 0)).xyz;

	normalCameraSpace = normalCamSpace;
	lightDirection = lightCamSpace - posCamSpace;
	toEye = -posCamSpace;


	distance = length(lightDirection);	
}