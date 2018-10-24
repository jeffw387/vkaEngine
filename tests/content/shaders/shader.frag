#version 450

const float gamma = 2.2;

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

layout (push_constant) uniform PushConstants {
  uint materialIndex;
} push;

layout (location = 0) in vec3 inViewPos;
layout (location = 1) flat in vec3 inViewNormal;

struct Material {
  vec4 diffuse;
};

struct Light {
  vec4 color;
  vec4 positionViewSpace;
};

layout (set = 0) readonly buffer Materials
{
  Material[] data;
} materials;

layout (set = 1) readonly buffer DynamicLights {
  Light[] data;
} dynamicLights;

layout (set = 2) uniform LightUniform {
  vec4 ambient;
  uint dynamicLightCount;
} lightUniform;

layout(location = 0) out vec4 outColor;

void main() {
  vec3 diffuseLighting = vec3(0,0,0);

  for (uint i = 0; i < lightUniform.dynamicLightCount; ++i) {
    vec3 lightViewPos = vec3(dynamicLights.data[i].positionViewSpace);
    vec3 surfaceToLight = lightViewPos - inViewPos;
    float cosTheta = dot(inViewNormal, surfaceToLight) 
      / (length(surfaceToLight) * length(inViewNormal));
    float intensity = (dynamicLights.data[i].color.a / length(surfaceToLight))
      * max(cosTheta, 0);
    diffuseLighting += dynamicLights.data[i].color.rgb * intensity;
  }
  vec3 diffuseMaterial = materials.data[push.materialIndex].diffuse.rgb;
  vec3 scaledAmbient = (lightUniform.ambient.rgb * lightUniform.ambient.a);
  vec3 hdrColor = diffuseMaterial + diffuseLighting + scaledAmbient;

  outColor = vec4(Uncharted2Tonemap(hdrColor), 1);
  // outColor = vec4(inViewNormal, 1);
  // outColor = vec4(dynamicLights.data[0].positionViewSpace.xyz - inViewPos, 1);
}