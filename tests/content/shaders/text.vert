#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

layout(push_constant) uniform uPushConstant{
  layout(offset=0) vec2 screenPos;
  layout(offset=8) vec2 clipSpaceScale;
  layout(offset=16) float textScale;
} pc;

out gl_PerVertex{
  vec4 gl_Position;
};

layout(location = 0) out struct{
  vec2 UV;
} Out;

void main() {
  Out.UV = aUV;
  vec2 scaledPos = (aPos * pc.textScale);
  vec2 translatedPos = scaledPos + pc.screenPos;
  vec2 clipPos = translatedPos * pc.clipSpaceScale;
  gl_Position = vec4(clipPos, 0, 1);
}
