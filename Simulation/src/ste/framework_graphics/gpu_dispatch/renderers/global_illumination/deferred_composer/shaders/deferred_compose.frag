
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "chromaticity.glsl"

#include "material.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "linked_light_lists.glsl"

#include "gbuffer.glsl"

#include "volumetric_scattering.glsl"

#include "project.glsl"
#include "girenderer_transform_buffer.glsl"

layout(std430, binding = 0) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 6) restrict readonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};

layout(r32ui, binding = 6) restrict readonly uniform uimage2D lll_heads;
layout(shared, binding = 11) restrict readonly buffer lll_data {
	lll_element lll_buffer[];
};

#include "light_load.glsl"
#include "linked_light_lists_load.glsl"
#include "gbuffer_load.glsl"

layout(binding = 8) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(binding = 9) uniform sampler3D scattering_volume;

out vec4 gl_FragColor;

vec3 shade(g_buffer_element frag) {
	int draw_idx = gbuffer_parse_material(frag);
	material_descriptor md = mat_descriptor[draw_idx];

	vec2 uv = gbuffer_parse_uv(frag);
	vec2 duvdx = gbuffer_parse_duvdx(frag);
	vec2 duvdy = gbuffer_parse_duvdy(frag);

	float depth = gbuffer_parse_depth(frag);
	vec3 position = unproject_screen_position(depth, gl_FragCoord.xy / vec2(backbuffer_size()));
	vec3 w_pos = dquat_mul_vec(view_transform_buffer.inverse_view_transform, position);

	vec3 n = gbuffer_parse_normal(frag);
	vec3 t = gbuffer_parse_tangent(frag);
	vec3 b = cross(t, n);
	normal_map(md, uv, duvdx, duvdy, n, t, b, position);

	vec3 diffuse_color = material_base_color(md, uv, duvdx, duvdy);
	float cavity = material_cavity(md, uv, duvdx, duvdy);

	vec3 rgb = material_emission(md);

	ivec2 lll_coords = ivec2(gl_FragCoord.xy) / lll_image_res_multiplier;
	uint32_t lll_ptr = imageLoad(lll_heads, lll_coords).x;
	for (;; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		if (lll_eof(lll_p))
			break;

		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		if (depth >= lll_depth_range.x) {
			uint light_idx = uint(lll_parse_light_idx(lll_p));
			light_descriptor ld = light_buffer[light_idx];

			vec3 incident = light_incidant_ray(ld, position);
			if (dot(n, incident) <= 0)
				continue;

			float light_effective_range = ld.effective_range;
			float dist2 = dot(incident,incident);
			if (dist2 >= light_effective_range*light_effective_range)
				continue;

			vec3 shadow_v = w_pos - ld.position;
			float shadow = shadow(shadow_depth_maps,
								  uint(lll_parse_ll_idx(lll_p)),
								  shadow_v);
			if (shadow <= .0f)
				continue;

			float dist = sqrt(dist2);
			float l_radius = ld.radius;

			vec3 v = normalize(-position);
			vec3 l = incident / dist;
			vec3 irradiance = light_irradiance(ld, dist) * shadow;

			rgb += material_evaluate_reflection(md,
												n, t, b,
												v, l,
												diffuse_color,
												cavity,
												irradiance);
		}
	}

	rgb = volumetric_scattering(scattering_volume, rgb, vec2(gl_FragCoord.xy), depth);

	return rgb;
}

void main() {
	g_buffer_element frag = gbuffer_load(ivec2(gl_FragCoord.xy));
	vec3 c = shade(frag);

	vec3 xyY = XYZtoxyY(RGBtoXYZ(c));
	gl_FragColor = vec4(xyY, 1);
}
