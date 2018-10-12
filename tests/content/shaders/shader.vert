#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (set = 0, binding = 3) uniform Camera {
  mat4 view;
  mat4 projection;
} camera;

layout (set = 0, binding = 4) uniform Instance {
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
