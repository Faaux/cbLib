#version 330 core

in vec2 uvCoords;
in vec3 normalCameraSpace;
in vec3 toEye;
in vec3 lightDirection;
in float distance;

out vec3 color;

uniform sampler2D albedo;
uniform sampler2D metallic;
uniform sampler2D roughness;

const vec3 lightColor = vec3(1.0,1.0,1.0);
const float lightPower = 20000;
const vec3 materialSpecular = vec3(1.0,1.0,1.0);
const float ambientStrength = 0.1;

void main()
{
	vec3 materialDiffuse = texture2D(metallic, uvCoords).rgb;

	vec3 n = normalize(normalCameraSpace);
	vec3 l = normalize(lightDirection);
	vec3 e = normalize(toEye);
	vec3 r = reflect(-l,n);

	float cosTheta = clamp(dot(n,l), 0, 1);
	float cosAlpha = clamp(dot(e,r), 0, 1);

	vec3 ambientColor = materialDiffuse * ambientStrength;
	vec3 diffuseColor = materialDiffuse * lightColor * lightPower * cosTheta / (distance*distance);
	vec3 specular = materialSpecular * lightColor * lightPower * pow(cosAlpha,5) / (distance*distance);
	color = ambientColor + diffuseColor + specular;
	//color = materialDiffuse;//vec3(uvCoords,0.0);
}