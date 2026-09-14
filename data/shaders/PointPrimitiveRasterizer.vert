#version 450 core

#SHADER_DEFINES // Here the VERTEX_SHADER_OUTPUT_XYZ get placed by the ShaderProgramFactory before compilation of the shader

uniform mat4 u_mvp;
uniform float u_point_size = 1.0;

in vec3 a_position;

#if defined(ATTRIBUTE_INPUT_POSITION)
    out vec3 v_position;
#endif

#if defined(ATTRIBUTE_INPUT_COLOR)
    in vec3 a_color;
    out vec3 v_color;
#endif

#if defined(ATTRIBUTE_INPUT_NORMAL)
    in vec3 a_normal;
    out vec3 v_normal;
#endif

#if defined(ATTRIBUTE_INPUT_ID)
    in uint a_id;
    out flat uint v_id;
#endif

#if defined(ATTRIBUTE_INPUT_SEGMENT)
    in uint a_segment;
    out flat uint v_segment;
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_FLOAT)
    in float a_custom;
    out float v_custom;
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_INT)
    in int a_custom;
    out flat int v_custom;
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_UINT)
    in uint a_custom;
    out flat uint v_custom;
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_VEC)
    in vec3 a_custom;
    out vec3 v_custom;
#endif


void main() {
    gl_Position = u_mvp * vec4(a_position, 1.0);
    //gl_PointSize = u_point_size * pow(1.1f, float(gl_BaseInstance));
    gl_PointSize = u_point_size;

    #if defined(ATTRIBUTE_INPUT_POSITION)
        v_position = a_position;
    #endif

    #if defined(ATTRIBUTE_INPUT_COLOR)
        v_color = a_color;
        //uint hashed = (uint(gl_BaseInstance) + 1u) * 2654435761u;
	    //v_color = vec3(float(hashed & 255u) / 255.0, float((hashed >> 8) & 255u) / 255.0, float((hashed >> 16) & 255u) / 255.0);
    #endif

    #if defined(ATTRIBUTE_INPUT_NORMAL)
        v_normal = a_normal;
    #endif

    #if defined(ATTRIBUTE_INPUT_ID)
        v_id = a_id;
    #endif

    #if defined(ATTRIBUTE_INPUT_SEGMENT)
        v_segment = a_segment;
    #endif

    #if defined(ATTRIBUTE_INPUT_CUSTOM_FLOAT) || defined(ATTRIBUTE_INPUT_CUSTOM_INT) || defined(ATTRIBUTE_INPUT_CUSTOM_UINT) || defined(ATTRIBUTE_INPUT_CUSTOM_VEC)
        v_custom = a_custom;
    #endif
}
