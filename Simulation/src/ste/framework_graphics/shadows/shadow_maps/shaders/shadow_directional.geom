
#type geometry
#version 450
#extension GL_NV_gpu_shader5 : require

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

#include "light.glsl"
#include "light_cascades.glsl"

#include "project.glsl"

#include "shadow.glsl"
#include "shadow_projection_instance_to_ll_idx_translation.glsl"

in vs_out {
	flat int instanceIdx;
	flat uint drawIdx;
} vin[];

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};
layout(shared, binding = 6) restrict readonly buffer directional_lights_cascades_data {
	light_cascade_descriptor directional_lights_cascades[];
};

layout(shared, binding = 8) restrict readonly buffer directional_shadow_projection_instance_to_ll_idx_translation_data {
	directional_shadow_projection_instance_to_ll_idx_translation dsproj_id_to_llid_tt[];
};

uniform float cascades_depths[directional_light_cascades];

vec3 transform(vec4 v, mat3x4 M) {
	return v * M;
}

void process(int cascade, uint32_t cascade_idx, vec3 vertices[3], float f) {
	// Cull triangles outside the NDC
	if ((vertices[0].x >  1 &&
		 vertices[1].x >  1 &&
		 vertices[2].x >  1) ||
		(vertices[0].x < -1 &&
		 vertices[1].x < -1 &&
		 vertices[2].x < -1) ||
		(vertices[0].y >  1 &&
		 vertices[1].y >  1 &&
		 vertices[2].y >  1) ||
		(vertices[0].y < -1 &&
		 vertices[1].y < -1 &&
		 vertices[2].y < -1) ||
		(vertices[0].z < -f &&
		 vertices[1].z < -f &&
		 vertices[2].z < -f))
		return;

	gl_Layer = cascade + int(cascade_idx) * directional_light_cascades;
	for (int j = 0; j < 3; ++j) {
		// Clamp z values behind the near-clip to the near-clip plane, this geometry participates in directional shadows as well.
		float z = min(-cascade_projection_near_clip, vertices[j].z);

		gl_Position.z = cascade_projection_near_clip;
		gl_Position.w = -z;
		// Orthographic projection
		gl_Position.xy = vertices[j].xy * gl_Position.w;

		EmitVertex();
	}

	EndPrimitive();
}

void main() {
	int sproj_instance_id = vin[0].instanceIdx;
	uint draw_id = vin[0].drawIdx;
	uint16_t ll_id = dsproj_id_to_llid_tt[draw_id].ll_idx[sproj_instance_id];

	light_descriptor ld = light_buffer[ll[ll_id]];

	// Calculate normal and cull back faces
	vec3 l = ld.transformed_position;
	vec3 u = gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 v = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 n = cross(u,v);

	if (dot(n,-l) <= 0)
		return;
		
	// Read cascade descriptor and per cascade build the transformation matrix, transform vertices and output
	uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
	light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];
	
	for (int cascade = 0; cascade < directional_light_cascades; ++cascade) {
		float z_far;
		mat3x4 M = light_cascade_projection(cascade_descriptor, cascade, l, cascades_depths, z_far);

		vec3 vertices[3];
		for (int j = 0; j < 3; ++j)
			vertices[j] = transform(gl_in[j].gl_Position, M);

		process(cascade, cascade_idx, vertices, z_far);
	}
}
