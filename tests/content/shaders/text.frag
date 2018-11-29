#version 450 core
layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform usampler2DArray sTexture;

layout(push_constant) uniform uPushConstant{
  layout(offset=32) vec4 fontColor;
  layout(offset=48) float distanceFactor;
  layout(offset=52) uint glyphIndex;
} pc;

layout(location = 0) in struct{
  vec2 UV;
} In;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
  vec3 msdfSample = texture(sTexture, vec3(In.UV, pc.glyphIndex)).rgb;
  float signedDistance = median(msdfSample.r, msdfSample.g, msdfSample.b);
  float pixelSignedDist = (pc.distanceFactor * (signedDistance - 0.5)) + 0.5;
  float opacity = smoothstep(0.5, 1, pixelSignedDist);
  // float opacity = clamp(pixelSignedDist, 0, 1);

  outColor = vec4(pc.fontColor.rgb, opacity);
}
