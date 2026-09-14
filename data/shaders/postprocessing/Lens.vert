#version 450

uniform float u_lens_radius;
uniform vec2 u_lens_position;
uniform float u_aspect_ratio;

out vec2 v_uv;

const float LENS_LABEL_HEIGHT = 0.15f;
const float LENS_LABEL_OFFSET = 0.005f;

void main()
{
    // Vertices:
    // 2 ------ 3
    // |        |
    // |        |
    // 0 ------ 1

    float v_x = mix(u_lens_position.x - u_lens_radius, u_lens_position.x + u_lens_radius, float(gl_VertexID & 1));
    float v_y = mix(u_lens_position.y + (u_lens_radius + LENS_LABEL_HEIGHT * u_lens_radius) * u_aspect_ratio + LENS_LABEL_OFFSET, u_lens_position.y - u_lens_radius * u_aspect_ratio, float((gl_VertexID >> 1) & 1));

    gl_Position = vec4(v_x, v_y, 0.0, 1.0);
    v_uv = vec2(v_x, v_y) * 0.5 + 0.5;
}