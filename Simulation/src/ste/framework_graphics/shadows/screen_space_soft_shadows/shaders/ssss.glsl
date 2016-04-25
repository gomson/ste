
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32) in;

const int light_buffers_first = 2;
#include "light.glsl"
#include "shadow.glsl"
#include "gbuffer.glsl"

layout(binding = 8) uniform samplerCubeArray shadow_depth_maps;
layout(r16f, binding = 0) uniform image2DArray penumbra_layers;
layout(r16f, binding = 1) uniform image2D z_buffer;

uniform float far, near;
uniform float half_over_tan_fov_over_two;
uniform mat4 inverse_view_matrix;
uniform mat4 transpose_view_matrix;

void main() {
	ivec2 coords = ivec2(vec2(gl_GlobalInvocationID.xy) / vec2(imageSize(penumbra_layers).xy) * gbuffer_size());

	g_buffer_element frag = gbuffer_load(coords);
	vec3 n = (transpose_view_matrix * vec4(frag.N, 1)).xyz;
	vec3 w_pos = (inverse_view_matrix * vec4(frag.P, 1)).xyz;
	float frag_depth = gbuffer_linear_z(frag, far, near);

	for (int i = 0; i < light_buffer.length(); ++i) {
		vec3 l_pos = light_buffer[i].position_direction.xyz;
		vec3 shadow_v = w_pos - l_pos;
		float d = dot(n, -shadow_v);

		float frag_out = .0f;

		if (d > .0f) {
			float l_radius = light_buffer[i].radius;
			float dist = length(w_pos - l_pos);

			bool shadowed;
			float w_penumbra = shadow_penumbra_width(shadow_depth_maps, i, shadow_v, l_radius, dist, far, shadowed);

			if (shadowed) {
				float anisotropy = dist / d;
				float d_screen = half_over_tan_fov_over_two;
				float w_screen_penumbra = w_penumbra * d_screen / frag_depth * anisotropy;

				frag_out = max(w_screen_penumbra, .05f);
			}
		}

		imageStore(penumbra_layers, ivec3(gl_GlobalInvocationID.xy, i), frag_out.xxxx);
	}

	imageStore(z_buffer, ivec2(gl_GlobalInvocationID.xy), frag_depth.xxxx);
}
