#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 normalCameraSpace;
out vec3 toEye;
out vec3 lightDirection;
out float distance;

const vec3 lightPos		=	vec3(-1.0,2.0,1.0);

void main()
{
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