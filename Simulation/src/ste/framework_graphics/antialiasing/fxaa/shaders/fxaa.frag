
#type frag
#version 450

#define FXAA_PRESET 3

#include <nvidia_fxaa.glsl>

in vec2 uv;

layout(location = 0) uniform sampler2D input_tex;

out vec4 frag_color;

void main() {
	vec2 rcp_frame = 1.f / textureSize(input_tex, 0);
	frag_color = vec4(FxaaPixelShader(uv, input_tex, rcp_frame), 1);
}
