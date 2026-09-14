uniform vec4 u_color;
uniform float u_original_color_factor;

vec4 colorization()
{
#if defined(USE_COLOR)
	return vec4(mix(u_color.rgb, getColor(), u_original_color_factor), u_color.a);
#else
	return u_color;
#endif
}