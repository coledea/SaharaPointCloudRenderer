#version 450 core

uniform float u_far_plane;
uniform float u_near_plane;
uniform float u_epsilon;
uniform vec4 u_added_tint;
uniform vec4 u_removed_tint;
uniform vec2 u_lens_position;
uniform float u_lens_radius;
uniform float u_lens_active;
uniform float u_lens_timestamp;

uniform sampler2D u_depth_texture_1;// original timestamp
uniform sampler2D u_color_texture_1;
uniform sampler2D u_depth_texture_2;// comparison timestamp
uniform sampler2D u_color_texture_2;

in vec2 v_uv;

out vec4 FragColor;

const float BORDER_THICKNESS = 0.04f * u_lens_radius;

float normalizeDepth(float depth)
{
    depth = depth * 2.0 - 1.0;
    depth = (2.0 * u_near_plane) / (u_far_plane + u_near_plane - depth * (u_far_plane - u_near_plane));// linearize and normalize depth
    return depth;
}

void main(void)
{
    const float half_border_size = BORDER_THICKNESS / 2.0;
    ivec2 texture_size = textureSize(u_color_texture_1, 0);
    ivec2 depth_coordinates = ivec2(v_uv * texture_size);
    float aspect_ratio = float(texture_size.x) / float(texture_size.y);

    // difference detection
    float depth_1 = texelFetch(u_depth_texture_1, depth_coordinates, 0).r;
    float depth_2 = texelFetch(u_depth_texture_2, depth_coordinates, 0).r;
    float depth_1_normalized = normalizeDepth(depth_1);
    float depth_2_normalized = normalizeDepth(depth_2);
    float difference = depth_2_normalized - depth_1_normalized;
    float differences_present = step(u_epsilon, abs(difference));// 0 if nothing changed, 1 otherwise
    float points_removed = step(0.0, difference);// 1 if something was removed, 0 otherwise
    float points_added = 1.0 - points_removed;// 1 if something was added, 0 otherwise

    // difference colorization
    vec4 color_1 = texture(u_color_texture_1, v_uv, 0);
    vec4 color_2 = texture(u_color_texture_2, v_uv, 0);
    float blend_factor = 0.6;
    vec4 difference_color =
    points_removed * (u_removed_tint * (1.0 - blend_factor) + color_1 * blend_factor)
    + points_added * (u_added_tint * (1.0 - blend_factor) + color_2 * blend_factor);

    vec4 difference_output_color = mix(color_1, difference_color, differences_present);
    float difference_output_depth = min(depth_1, depth_2);

    // lens colorization
    float distance_to_lens = length((u_lens_position - (v_uv * 2.0 - 1.0)) / vec2(1.0, aspect_ratio));
    float in_lens = step(distance_to_lens + half_border_size, u_lens_radius);

    vec4 lens_color = mix(color_1, color_2, u_lens_timestamp);
    float lens_output_depth = mix(depth_1, depth_2, u_lens_timestamp);

    FragColor = mix(difference_output_color, lens_color, in_lens * u_lens_active);
    gl_FragDepth = mix(difference_output_depth, lens_output_depth, in_lens * u_lens_active);
}
