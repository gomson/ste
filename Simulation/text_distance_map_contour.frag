
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

struct buffer_glyph_descriptor {
	int16_t width;
	int16_t height;
	int16_t start_y;
	int16_t start_x;
	uint64_t tex_handler;
};

out vec4 gl_FragColor;

in geo_out {
	vec4 color;
	vec2 st;
	flat int drawId;
	float weight;
} vin;

layout(std430, binding = 0) buffer glyph_data {
	buffer_glyph_descriptor glyphs[];
};

float aastep (float threshold , float value) {
	float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
	return smoothstep(threshold-afwidth, threshold+afwidth, value);
}

void main( void ) {
	buffer_glyph_descriptor glyph = glyphs[vin.drawId];

	vec2 uv = vin.st;
	float D = textureLod(sampler2D(glyph.tex_handler), uv, 0).x;

	D -= vin.weight;

    float g = 1.0f - aastep(0.0, D);

	vec4 c = vin.color * vec4(1, 1, 1, g);
	gl_FragColor = c;
}

