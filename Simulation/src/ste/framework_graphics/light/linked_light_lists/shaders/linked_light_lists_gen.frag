
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "common.glsl"

#include "light.glsl"
#include "linked_light_lists.glsl"
#include "linked_light_lists_store.glsl"

#include "girenderer_transform_buffer.glsl"

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};

layout(r32ui, binding = 6) restrict writeonly uniform uimage2D lll_heads;
layout(shared, binding = 7) restrict buffer lll_counter_data {
	uint32_t lll_counter;
};
layout(shared, binding = 11) restrict writeonly buffer lll_data {
	lll_element lll_buffer[];
};

layout(bindless_sampler) uniform sampler2D depth_map;

void main() {
	ivec2 image_coord = ivec2(gl_FragCoord.xy);
	vec2 frag_coords = (vec2(gl_FragCoord.xy) + vec2(.5f)) / vec2(backbuffer_size()) * float(lll_image_res_multiplier);

	int depth_lod = 2;

	float near = projection_near_clip();
	vec2 ndc = frag_coords * 2.f - vec2(1.f);

	float near_plane_h = projection_tan_half_fovy() * near;
	float near_plane_w = near_plane_h * projection_aspect();
	vec2 near_plane_pos = ndc * vec2(near_plane_w, near_plane_h);

	vec3 l = vec3(near_plane_pos, -near);
	float a = dot(l, l);

	lll_element active_lights[max_active_lights_per_frame];
	int total_active_lights = 0;

	for (int j = 0; j < ll_counter && total_active_lights < max_active_lights_per_frame; ++j) {
		uint16_t ll_i = uint16_t(j);
		uint16_t light_idx = ll[ll_i];
		light_descriptor ld = light_buffer[light_idx];
		
		float depth_zmin;
		float depth_zmax;
		bool add_point = false;

		if (ld.type == LightTypeDirectional) {
			// For directional lights: Nothing to do, add point
			depth_zmin = project_depth(-inf);
			depth_zmax = project_depth(-near);

			add_point = true;
		}
		else {
			// For spherical lights: Check to see that fragment's stored depth intersects the lights cutoff sphere.
			vec3 c = ld.transformed_position;
			float r = ld.effective_range * 1.05f;

			vec3 _c = -c;
			float b = dot(l, _c);

			float delta = b*b - a * (dot(c, c) - r*r);
			if (delta > 0) {
				float sqrt_delta = sqrt(delta);
				float z_max = l.z * (-b - sqrt_delta) / a;
				float z_min = l.z * (-b + sqrt_delta) / a;

				depth_zmin = clamp(project_depth(z_min), .0f, 1.f);
				depth_zmax = project_depth(z_max);

				if (z_min < -near) {
					if (z_max >= -near) {
						// Origin is inside the light radius
						depth_zmax = 1.f;
						add_point = true;
					}
					else {
						// Compare against depth buffer
						float d = textureLod(depth_map, (vec2(image_coord) + vec2(.5f)) / textureSize(depth_map, depth_lod).xy, depth_lod).x;
						if (d <= depth_zmax)
							add_point = true;
					}
				}
			}
		}

		if (add_point) {
			active_lights[total_active_lights] = lll_encode(light_idx,
															ll_i,
															depth_zmin,
															depth_zmax);

			++total_active_lights;
		}
	}

	// Add the encoded lights to the per-pixel linked-light-list
	uint32_t next_idx = atomicAdd(lll_counter, uint(total_active_lights + 1));
	imageStore(lll_heads, image_coord, next_idx.xxxx);

	for (int i = 0; i < total_active_lights; ++i)
		lll_buffer[next_idx + i] = active_lights[i];
	// And mark the ll end
	lll_buffer[next_idx + total_active_lights].data.x = uintBitsToFloat(0xFFFFFFFF);
}
