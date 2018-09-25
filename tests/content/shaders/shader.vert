#version 450

//layout (constant_id = 0) const uint materialCount;
//layout (constant_id = 1) const uint lightCount;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

// struct Material {
//   vec4 diffuse;
// };

// struct Light {
//   vec4 color;
//   vec4 positionViewSpace;
// };

// layout (set = 0, binding = 0) uniform Materials
// {
//   Material[materialCount] data;
// } materials;

// layout (set = 0, binding = 1) uniform Lights {
//   Light[lightCount] data;
// } lights;

layout (set = 0, binding = 2) uniform Camera {
  mat4 view;
  mat4 projection;
} camera;

layout (set = 0, binding = 3) uniform Instance {
  mat4 model;
} instance;

layout (location = 0) out vec3 outViewPos;
layout (location = 1) flat out vec3 outViewNormal;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
  mat4 viewModel = camera.view * instance.model;
  outViewPos =  vec3(viewModel * vec4(inPos, 1));
  outViewNormal = normalize(vec3(viewModel * vec4(inNormal, 1)));
	gl_Position =  camera.projection * viewModel * vec4(inPos, 1.0);
}
