#version 460 core

#SHADER_DEFINES // Here the VERTEX_SHADER_OUTPUT_XYZ get placed by the ShaderProgramFactory before compilation of the shader

#if defined(VERTEX_SHADER_OUTPUT_COLOR)
    in vec3 v_color;
#endif

#if defined(VERTEX_SHADER_OUTPUT_POSITION)
    in vec3 v_position;
#endif

#if defined(VERTEX_SHADER_OUTPUT_NORMAL)
    in vec3 v_normal;
#endif

#if defined(VERTEX_SHADER_OUTPUT_ID)
    in flat uint v_id;
#endif

#if defined(VERTEX_SHADER_OUTPUT_SEGMENT)
    in flat uint v_segment;
#endif

#if defined(VERTEX_SHADER_OUTPUT_CUSTOM_FLOAT)
    in float v_custom;
#endif

#if defined(VERTEX_SHADER_OUTPUT_CUSTOM_INT)
    in flat int v_custom;
#endif

#if defined(VERTEX_SHADER_OUTPUT_CUSTOM_UINT)
    in flat uint v_custom;
#endif

#if defined(VERTEX_SHADER_OUTPUT_CUSTOM_VEC)
    in vec3 v_custom;
#endif

out vec4 fragColor;

// The order of these outputs must correspond to the order of utils::FramebufferAttachmentTypes
#if defined(FRAGMENT_SHADER_OUTPUT_NORMAL)
    out vec4 o_normal;
#endif

#if defined(FRAGMENT_SHADER_OUTPUT_POSITION)
    out vec4 o_position;
#endif

#if defined(FRAGMENT_SHADER_OUTPUT_SEGMENT)
    out uint o_segment;
#endif

#if defined(FRAGMENT_SHADER_OUTPUT_ID)
    out uint o_id;
#endif

#if defined(FRAGMENT_SHADER_OUTPUT_CUSTOM_FLOAT)
    out float o_custom;
#endif

#if defined(FRAGMENT_SHADER_OUTPUT_CUSTOM_INT)
    out int o_custom;
#endif

#if defined(FRAGMENT_SHADER_OUTPUT_CUSTOM_UINT)
    out uint o_custom;
#endif

#if defined(FRAGMENT_SHADER_OUTPUT_CUSTOM_VEC)
    out vec3 o_custom;
#endif

#COLORIZATION

void main(void)
{
    #if defined(VERTEX_SHADER_OUTPUT_COLOR)
        fragColor = colorization(vec4(v_color, 1.0));
    #else
        fragColor = colorization(vec4(1.0));
    #endif

    #if defined(FRAGMENT_SHADER_OUTPUT_POSITION)
        o_position = vec4(v_position, 1.0);
    #endif

     #if defined(FRAGMENT_SHADER_OUTPUT_NORMAL)
        o_normal = vec4(v_normal, 1.0);
    #endif

    #if defined(FRAGMENT_SHADER_OUTPUT_ID)
        o_id = v_id;
    #endif

    #if defined(FRAGMENT_SHADER_OUTPUT_SEGMENT)
        o_segment = v_segment;
    #endif

    #if defined(FRAGMENT_SHADER_OUTPUT_CUSTOM_FLOAT) || defined(FRAGMENT_SHADER_OUTPUT_CUSTOM_INT) || defined(FRAGMENT_SHADER_OUTPUT_CUSTOM_UINT) || defined(FRAGMENT_SHADER_OUTPUT_CUSTOM_VEC)
        o_custom = v_custom;
    #endif

}