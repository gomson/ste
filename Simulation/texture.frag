
#type frag
#version 440

in vec2 tex_coords;

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D tex;

void main() {
    gl_FragColor = vec4(texture(tex, tex_coords).rgb, 1);
}
