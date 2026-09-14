#version 450 core

uniform float u_thickness;
uniform float u_xposition;

void main() {
    // Vertices:
    // 2 ------ 3
    // |        |
    // |        |
    // 0 ------ 1

    float half_thickness = u_thickness / 2.0;
    float v_x = mix(u_xposition - half_thickness, u_xposition + half_thickness, float(gl_VertexID & 1));
    float v_y = mix(-1.0, 1.0, float((gl_VertexID >> 1) & 1));

    gl_Position = vec4(v_x, v_y, 0.0, 1.0);
}