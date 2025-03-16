#version 140
uniform mediump sampler2D texture_sampler;
in mediump vec2 var_texcoord0;
out vec4 out_fragColor;

void main() {
	out_fragColor = texture(texture_sampler, var_texcoord0.xy);
}