#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

layout(push_constant) uniform uPushConstant{
    mat4 mvp;
} pc;

out gl_PerVertex{
    vec4 gl_Position;
};

layout(location = 0) out struct{
    vec2 UV;
} Out;

void main()
{
    Out.UV = aUV;
    gl_Position = pc.mvp * vec4(aPos, 0, 1);
}
