#version 450 core
layout (constant_id = 0) const uint GLYPHCOUNT = 1;
layout (constant_id = 1) const vec2 MSDFSIZE = vec2(32, 32);

layout(location = 0) out vec4 outColor;

layout(set=0) uniform sampler2D sTexture[GLYPHCOUNT];
layout(set=1) uniform Material {
  vec4 color;
} material;

layout(push_constant) uniform uPushConstant{
  layout(offset=4) uint glyphIndex;
  layout(offset=8) float pxRange;
  layout(offset=32) vec2 clipSpaceScale;
} pc;

layout(location = 0) in struct{
  vec2 UV;
} In;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
  vec2 msdfUnit = pxRange/MSDFSIZE;
  vec3 sample = texture(sTexture[glyphIndex], In.UV).rgb;
  float signedDistance = median(sample.r, sample.g, sample.b) - 0.5;
  float signedDistanceInPixels = signedDistance * length(clipSpaceScale);
  float opacity = clamp(signedDistanceInPixels + 0.5, 0.0, 1.0);

  outColor = vec4(material.color.rgb, opacity) ;
}
