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

uniform sampler2D u_change_texture_1;// original timestamp
uniform sampler2D u_color_texture_1;
uniform sampler2D u_depth_texture_1;
uniform sampler2D u_change_texture_2;// comparison timestamp
uniform sampler2D u_color_texture_2;
uniform sampler2D u_depth_texture_2;

in vec2 v_uv;

out vec4 FragColor;

const float BORDER_THICKNESS = 0.04f * u_lens_radius;

void main(void)
{
    const float half_border_size = BORDER_THICKNESS / 2.0;
    ivec2 texture_size = textureSize(u_color_texture_1, 0);
    ivec2 texcoords = ivec2(v_uv * texture_size);
    float aspect_ratio = float(texture_size.x) / float(texture_size.y);

    float depth_1 = texelFetch(u_depth_texture_1, texcoords, 0).r;
    float depth_2 = texelFetch(u_depth_texture_2, texcoords, 0).r;
    float difference_output_depth = min(depth_1, depth_2);

    // difference detection
    float change_1 = abs(texelFetch(u_change_texture_1, texcoords, 0).r);
    float change_2 = abs(texelFetch(u_change_texture_2, texcoords, 0).r); 

    float relevant_timestamp = step(0.0, depth_1 - depth_2);
    float points_removed = step(u_epsilon, change_1) * (1.0 - relevant_timestamp);
    float points_added = step(u_epsilon, change_2) * relevant_timestamp;
    float differences_present = step(0.1, points_removed + points_added);

    // difference colorization
    vec4 color_1 = texture(u_color_texture_1, v_uv, 0);
    vec4 color_2 = texture(u_color_texture_2, v_uv, 0);
    float blend_factor = 0.60;
    vec4 difference_color =
    points_removed * (u_removed_tint * (1.0 - blend_factor) + color_1 * blend_factor)
    + points_added * (u_added_tint * (1.0 - blend_factor) + color_2 * blend_factor);

    vec4 original_color = color_1; // if there are points in only one of the timestamps show those in their original color
    vec4 difference_output_color = mix(original_color, difference_color, differences_present);
    difference_output_depth = mix(depth_1, difference_output_depth, differences_present);

    // focus colorization
    float distance_to_lens = length(vec2(u_lens_position - (v_uv * 2.0 - 1.0)) / vec2(1.0, aspect_ratio));
    float in_lens = step(distance_to_lens + half_border_size, u_lens_radius);

    vec4 lens_color = mix(color_1, color_2, u_lens_timestamp);
    float lens_output_depth = mix(depth_1, depth_2, u_lens_timestamp);

    FragColor = mix(difference_output_color, lens_color, in_lens * u_lens_active);
    gl_FragDepth = mix(difference_output_depth, lens_output_depth, in_lens * u_lens_active);
}
