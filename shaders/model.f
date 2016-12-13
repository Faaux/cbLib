#version 330 core

in vec3 normalWorld;
in vec3 lightDirection;
in float distance;

out vec3 color;

const vec3 lightColor = vec3(1.0,0.0,0.0);
const float lightPower = 2.0;
const vec3 materialDiffuse = vec3(0.5,0.0,1.0);
const float ambientStrength = 0.1;

void main()
{
	vec3 n = normalize( normalWorld );
	vec3 l = normalize( lightDirection );

	float cosTheta = clamp( dot( n,l ), 0,1 );

	vec3 ambientColor = materialDiffuse * ambientStrength;
	vec3 diffuseColor = materialDiffuse * lightColor * lightPower * cosTheta / (distance*distance);
	color = ambientColor + diffuseColor;
}