#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3 Normal;

layout(location = 0) out vec4 outColor;

struct DirLight
{
	vec3 diffuse;

	float specular;
	float intensity;
	
	vec3 ambient;
	vec3 direction;
};

layout(binding = 0) uniform UboScene
{
	mat4 view;
	mat4 proj;
	vec3 viewPos;
} ubo;

layout(binding = 1) uniform UboObject 
{
	vec4 color;
	float shininess;
} object;

layout(binding = 2) uniform Lights
{
	uint lightNum;
	DirLight dirLight;
} lights;

layout(binding = 3) uniform sampler2D diffuseMap;

vec3 GetDiffuse()
{
	vec3 diffuse = object.color.rgb;
	diffuse *= texture(diffuseMap, TexCoords).rgb;

	return diffuse;
}

float GetSpecular()
{
	return 1.0;
}

vec3 GetEmissive()
{
	return vec3(0.0);
}

vec3 CalcNoLight()
{
    return GetDiffuse();
}

vec3 CalcDirLight(vec3 viewDir)
{
	vec3 ambient = vec3(lights.dirLight.ambient) * GetDiffuse();
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(-vec3(lights.dirLight.direction));

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = vec3(lights.dirLight.diffuse) * diff * GetDiffuse();

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), object.shininess);

	vec3 specular = vec3(lights.dirLight.diffuse) * lights.dirLight.specular * spec * GetSpecular();

	diffuse *= lights.dirLight.intensity;
	specular *= lights.dirLight.intensity;

	return ambient + diffuse + specular;
}

void main() 
{
	vec3 lighting;
	vec3 viewDir = normalize(ubo.viewPos - FragPos);
	
	if (lights.lightNum != 0u)
	{
		lighting = CalcDirLight(viewDir);
	}
	else
	{
		lighting = CalcNoLight();
	}
	
	lighting += GetEmissive();

	outColor = vec4(lighting, 1.0);
}