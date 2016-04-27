
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require
// #extension GL_NV_shader_atomic_fp16_vector : require

#include "shadow.glsl"
#include "material.glsl"
#include "light.glsl"
#include "linked_light_lists.glsl"
#include "gbuffer.glsl"
//#include "voxels.glsl"

layout(std430, binding = 0) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(std430, binding = 3) restrict readonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};

layout(std430, binding = 6) restrict readonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};
layout(r32ui, binding = 7) restrict readonly uniform uimage2D gbuffer_ll_heads;

layout(r32ui, binding = 6) restrict readonly uniform uimage2D lll_heads;
layout(std430, binding = 11) restrict readonly buffer lll_data {
	lll_element lll_buffer[];
};

#include "light_load.glsl"
#include "linked_light_lists_load.glsl"
#include "gbuffer_load.glsl"

layout(binding = 8) uniform samplerCubeArrayShadow shadow_depth_maps;

in vs_out {
	vec2 tex_coords;
} vin;

out vec4 gl_FragColor;

uniform float scattering_ro = 0.0003f;
uniform mat4 inverse_view_matrix;
uniform float proj22, proj23;

vec4 shade(g_buffer_element frag) {
	uint16_t draw_idx = frag.material;
	vec4 c = frag.albedo;

	vec3 diffuse = c.rgb;
	float alpha = c.a;
	float specular = mix(.3f, 1.f, frag.specular);

	if (draw_idx == material_none)
		return vec4(diffuse, alpha);

	material_descriptor md = mat_descriptor[draw_idx];

	vec3 n = frag.N;
	vec3 t = frag.T;
	vec3 b = cross(t, n);
	vec3 position = frag.P.xyz;
	vec3 w_pos = (inverse_view_matrix * vec4(position, 1)).xyz;
	vec3 rgb = md.emission.rgb;

	ivec2 lll_coords = ivec2(gl_FragCoord.xy) / 4;

	uint32_t lll_next_ptr = imageLoad(lll_heads, lll_coords).x;
	for (int i = 0; i < max_active_lights_per_frame && !lll_eof(lll_next_ptr); ++i) {
		lll_element l = lll_buffer[lll_next_ptr];

		if (position.z >= l.z_min && position.z <= l.z_max) {
			int light_idx = int(l.light_idx);
			light_descriptor ld = light_buffer[light_idx];

			vec3 v = light_incidant_ray(ld, light_idx, position);
			if (dot(n, v) <= 0)
				continue;

			float dist = length(v);
			float l_radius = ld.radius;
			vec3 l = diffuse * ld.diffuse.xyz;
			float min_lum = light_min_effective_luminance(ld);

			vec3 shadow_v = w_pos - ld.position_direction.xyz;
			float shadow = shadow_penumbra_width(shadow_depth_maps, light_idx, shadow_v, l_radius, dist, proj22, proj23);

			float dist_att = dist * scattering_ro;
			float shadow_attenuation = 1.f - exp(-dist_att * dist_att);
			float obscurance = mix(1.f, .05f * shadow_attenuation, shadow);

			float brdf = calc_brdf(md, position, n, t, b, v);
			float attenuation_factor = light_attenuation_factor(ld, dist);
			float incident_radiance = max(ld.luminance / attenuation_factor - min_lum, .0f);

			float irradiance = specular * brdf * incident_radiance * obscurance;
			rgb += l * max(0.f, irradiance);
		}

		lll_next_ptr = l.next_ptr;
	}

	return vec4(rgb, alpha);
}

void main() {
	g_buffer_element frag = gbuffer_load(gbuffer_ll_heads, ivec2(gl_FragCoord.xy));
	vec4 c = shade(frag);

	int i = 0;
	while (!gbuffer_eof(frag.next_ptr) && i++ < 5) {
		frag = gbuffer_load(frag.next_ptr);
		vec4 c2 = shade(frag);

		c = c * c.a + c2 * (1.f - c.a);
	}

	vec3 xyY = XYZtoxyY(RGBtoXYZ(c.rgb));
	xyY.z = max(min_luminance, xyY.z);

	gl_FragColor = vec4(xyY, 1);
}
