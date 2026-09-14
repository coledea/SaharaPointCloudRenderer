#ifdef USE_DEPTH
	uniform float u_near_plane;
	uniform float u_far_plane;
#endif

#ifdef USE_POSITION
	uniform vec3 u_bbox_min;
	uniform vec3 u_bbox_max;
#endif

#ifdef USE_ID
	uniform uint u_number_of_points;
#endif

#ifdef USE_CUSTOM_FLOAT
	uniform float u_custom_value_min;
	uniform float u_custom_value_max;
#endif

#ifdef USE_CUSTOM_INT
	uniform int u_custom_value_min;
	uniform int u_custom_value_max;
#endif

#ifdef USE_CUSTOM_UINT
	uniform uint u_custom_value_min;
	uniform uint u_custom_value_max;
#endif

#ifdef USE_CUSTOM_VEC
	uniform vec3 u_custom_value_min;
	uniform vec3 u_custom_value_max;
#endif

uniform bool u_use_linear_color_scale;
uniform float u_original_color_factor;

vec3 qualitativeColorForValue(uint value)
{
	uint hashed = (value + 1u) * 2654435761u;
	return vec3(float(hashed & 255u) / 255.0, float((hashed >> 8) & 255u) / 255.0, float((hashed >> 16) & 255u) / 255.0); // ignoring the hightest order Byte for now
}

vec4 colorization()
{
	vec4 return_color = vec4(0.0, 0.0, 0.0, 1.0);

#if defined(USE_COLOR)
	vec4 original_color = vec4(getColor(), 1.0);
	return_color = original_color;
#endif

#if defined(USE_DEPTH)
	float depth = getDepth() * 2.0 - 1.0;
	depth = (2.0 * u_near_plane * u_far_plane) / (u_far_plane + u_near_plane - depth * (u_far_plane - u_near_plane));  // linearize depth
	return_color.rgb = vec3(depth / u_far_plane);
#elif defined(USE_NORMAL)
	return_color.rgb = getNormal() * 0.5 + 0.5;
#elif defined(USE_POSITION)
	return_color.rgb = (getPosition() - u_bbox_min) / (u_bbox_max - u_bbox_min);
#elif defined(USE_ID)
	const uint stepsize = 10000000;
	float greyscale = 0.5 - cos(float(double(getId() % stepsize) / double(stepsize)) * 2.0f * 3.141592653589793f) * 0.5f; // use periodic function -> makes it easier to spot point order/locality mismatches
	return_color.rgb = vec3(greyscale);
#elif defined(USE_SEGMENT)
	return_color.rgb = qualitativeColorForValue(getSegment());
#elif defined(USE_CUSTOM_FLOAT)
	return_color.rgb = vec3((getCustom() - u_custom_value_min) / (u_custom_value_max - u_custom_value_min));
#elif defined(USE_CUSTOM_INT)
	if (u_use_linear_color_scale)
		return_color.rgb = vec3(float(getCustom() - u_custom_value_min) / float(u_custom_value_max - u_custom_value_min));
	else
		return_color.rgb = qualitativeColorForValue(uint(getCustom()));
#elif defined(USE_CUSTOM_UINT)
	if (u_use_linear_color_scale)
		return_color.rgb = vec3(float(getCustom() - u_custom_value_min) / float(u_custom_value_max - u_custom_value_min));
	else
		return_color.rgb = qualitativeColorForValue(getCustom());
#elif defined(USE_CUSTOM_VEC)
	return_color.rgb = (getCustom() - u_custom_value_min) / (u_custom_value_max - u_custom_value_min);
#endif

#if defined(USE_COLOR)
	return mix(return_color, original_color, u_original_color_factor);
#else
	return return_color;
#endif
}