#version 330 core

in vec2 uvCoords;
in vec3 normalCameraSpace;
in vec3 toEye;
in vec3 lightDirection;
in float distance;

out vec3 color;

uniform sampler2D albedoTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;
uniform sampler2D iblbrdf;

const vec3 lightColor = vec3(1.0,1.0,1.0);
const float lightPower = 20000;
const vec3 materialSpecular = vec3(1.0,1.0,1.0);
const float ambientStrength = 0.1;

#define PI 3.1415926

float phong_diffuse()
{
    return (1.0 / PI);
}

vec3 phong_specular(in vec3 V, in vec3 L, in vec3 N, in vec3 specular, in float roughness)
{
    vec3 R = reflect(-L, N);
    float spec = max(0.0, dot(V, R));

    float k = 1.999 / (roughness * roughness);

    return min(1.0, 3.0 * 0.0398 * k) * pow(spec, min(10000.0, k)) * specular;
}

vec3 fresnel_factor(in vec3 f0, in float product)
{
    return mix(f0, vec3(1.0), pow(1.01 - product, 5.0));
}

void main()
{
	float A = 20.0 / dot(lightDirection, lightDirection);

	vec3 nn = normalize(normalCameraSpace);
	vec3 L = normalize(lightDirection);
	vec3 V = normalize(toEye);
	vec3 H = normalize(L + V);

	vec3 N = nn;

	vec3 base = texture2D(albedoTex, uvCoords).xyz;

	float metallic = texture2D(metallicTex, uvCoords).x;
	float roughness = texture2D(roughnessTex, uvCoords).x;

	vec3 specular = mix(vec3(0.04), base, metallic);


	float NdL = max(0.0, dot(N, L));
    float NdV = max(0.001, dot(N, V));
    float NdH = max(0.001, dot(N, H));
    float HdV = max(0.001, dot(H, V));
    float LdV = max(0.001, dot(L, V));

	vec3 specfresnel = fresnel_factor(specular, NdV);
    vec3 specref = phong_specular(V, L, N, specfresnel, roughness);

	specref *= vec3(NdL);

    // diffuse is common for any model
    vec3 diffref = (vec3(1.0) - specfresnel) * phong_diffuse() * NdL;

	 // compute lighting
    vec3 reflected_light = vec3(0);
    vec3 diffuse_light = vec3(0); // initial value == constant ambient light

	// point light
	vec3 light_color = vec3(1.0) * A;
    reflected_light += specref * light_color;
    diffuse_light += diffref * light_color;

	// IBL lighting
    vec2 brdf = texture2D(iblbrdf, vec2(roughness, 1.0 - NdV)).xy;
    vec3 iblspec = min(vec3(0.99), fresnel_factor(specular, NdV) * brdf.x + brdf.y);
    //reflected_light += iblspec * vec3(0.7,0.7,0.7);
    diffuse_light += vec3(0.0,0,0) * (1.0 / PI);

	vec3 result = diffuse_light * mix(base, vec3(0.0), metallic) + reflected_light;

	color = result;
}