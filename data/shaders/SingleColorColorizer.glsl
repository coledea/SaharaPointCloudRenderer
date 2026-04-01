uniform vec4 u_color;
uniform float u_original_color_factor;

vec4 colorization(vec4 color)
{
	return vec4(mix(u_color.rgb, color.rgb, u_original_color_factor), u_color.a * color.a);
}