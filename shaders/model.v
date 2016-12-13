#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 normalWorld;
out vec3 lightDirection;
out float distance;

const vec3 lightPos		=	vec3(-1.0,2.0,1.0);

void main()
{
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(position, 1);

	vec3 posWorldSpace = (modelMatrix * vec4(position, 1)).xyz;
	vec3 lightWorldSpace = lightPos;
	normalWorld = (modelMatrix * vec4(normal, 1)).xyz;
	lightDirection = lightWorldSpace - posWorldSpace;


	distance = length(lightDirection);
	
}