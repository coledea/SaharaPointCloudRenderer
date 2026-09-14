#version 450

uniform vec2 u_lens_position;
uniform float u_lens_radius;
uniform float u_lens_timestamp;
uniform float u_aspect_ratio;

in vec2 v_uv;

out vec4 FragColor;

const float BORDER_THICKNESS = 0.04f * u_lens_radius;
const float BLEND_RADIUS = 0.2;
const float LENS_LABEL_WIDTH = 0.1f;
const float LENS_LABEL_HEIGHT = 0.15f;
const float LENS_LABEL_OFFSET = 0.005f;
const float SEGMENT_THICKNESS = u_lens_radius * LENS_LABEL_WIDTH / 5.0;

// tests whether point p is inside the rectangle defined by center and half_size
float isInRectangle(vec2 p, vec2 center, vec2 half_size)
{
    vec2 d = abs(p - center) - half_size;
    float distance_to_rectangle = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
    return 1.0 - smoothstep(0.0, SEGMENT_THICKNESS * BLEND_RADIUS, distance_to_rectangle);
}

// tests whether a point p is inside the digit label for the given timestamp
float isInLabel(vec2 p, vec2 label_center, float timestamp)
{
    float half_width = u_lens_radius * LENS_LABEL_WIDTH * 0.5;
    float half_height = u_lens_radius * LENS_LABEL_HEIGHT * 0.5 * u_aspect_ratio;

    float digit_one = isInRectangle(p, label_center, vec2(SEGMENT_THICKNESS, half_height));

    float top = isInRectangle(p, label_center + vec2(0.0, half_height - SEGMENT_THICKNESS), vec2(half_width, SEGMENT_THICKNESS));
    float middle = isInRectangle(p, label_center, vec2(half_width, SEGMENT_THICKNESS));
    float bottom = isInRectangle(p, label_center + vec2(0.0, -half_height + SEGMENT_THICKNESS), vec2(half_width, SEGMENT_THICKNESS));
    float upper_right = isInRectangle(p, label_center + vec2(half_width - SEGMENT_THICKNESS, half_height * 0.5), vec2(SEGMENT_THICKNESS, half_height * 0.5));
    float lower_left = isInRectangle(p, label_center + vec2(-half_width + SEGMENT_THICKNESS, -half_height * 0.5), vec2(SEGMENT_THICKNESS, half_height * 0.5));
    float digit_two = max(max(max(top, middle), bottom), max(upper_right, lower_left));

    return mix(digit_one, digit_two, step(0.5, timestamp));
}

void main() {
    vec2 fragment_position = v_uv * 2.0 - 1.0;
    float distance_fragment_lens = length(vec2(u_lens_position - fragment_position) / vec2(1.0, u_aspect_ratio));

    vec4 border_color = vec4(0.0, 0.0, 0.0, 1.0);

    float border = 1.0 - smoothstep(u_lens_radius - BORDER_THICKNESS * BLEND_RADIUS, u_lens_radius, distance_fragment_lens);
    FragColor = mix(vec4(0.0), border_color, border);

    float in_lens = smoothstep(u_lens_radius - BORDER_THICKNESS * (1.0 - BLEND_RADIUS), u_lens_radius - BORDER_THICKNESS, distance_fragment_lens);
    FragColor = mix(FragColor, vec4(0.0), in_lens);

    vec2 label_center = u_lens_position
        + vec2(0.0, (u_lens_radius + LENS_LABEL_HEIGHT * u_lens_radius * 0.5) * u_aspect_ratio + LENS_LABEL_OFFSET);
    float label_mask = isInLabel(fragment_position, label_center, u_lens_timestamp);
    FragColor = mix(FragColor, vec4(0.0, 0.0, 0.0, 1.0), label_mask);
}
