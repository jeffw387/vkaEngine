#version 450 core
layout(location = 0) out vec4 outColor;

layout(set=0) uniform sampler2DArray sTexture;

layout(push_constant) uniform uPushConstant{
  layout(offset=0) float textScale;
  layout(offset=4) uint glyphIndex;
  layout(offset=8) vec2 screenPos;
  layout(offset=16) vec2 clipSpaceScale;
  layout(offset=32) vec4 fontColor;
} pc;

layout(location = 0) in struct{
  vec2 UV;
} In;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
  vec3 sample = texture(sTexture, vec3(In.UV, pc.glyphIndex)).rgb;
  float signedDistance = median(sample.r, sample.g, sample.b) - 0.5;
  float signedDistanceInPixels = signedDistance * length(clipSpaceScale);
  float opacity = clamp(signedDistanceInPixels + 0.5, 0.0, 1.0);

  outColor = vec4(pc.fontColor.rgb, opacity) ;
}
