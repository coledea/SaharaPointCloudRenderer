#version 450 core

#SHADER_DEFINES // Here the VERTEX_SHADER_OUTPUT_XYZ get placed by the ShaderProgramFactory before compilation of the shader

#if defined(ATTRIBUTE_INPUT_COLOR)
    in vec3 v_color;

    vec3 getColor()
    {
        return v_color;
    }
#endif

#if defined(ATTRIBUTE_INPUT_POSITION)
    in vec3 v_position;

    vec3 getPosition()
    {
        return v_position;
    }
#endif

#if defined(ATTRIBUTE_INPUT_NORMAL)
    in vec3 v_normal;

    vec3 getNormal()
    {
        return v_normal;
    }
#endif

#if defined(ATTRIBUTE_INPUT_ID)
    in flat uint v_id;

    uint getId()
    {
        return v_id;
    }
#endif

#if defined(ATTRIBUTE_INPUT_SEGMENT)
    in flat uint v_segment;

    uint getSegment()
    {
        return v_segment;
    }
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_FLOAT)
    in float v_custom;

    float getCustom()
    {
        return v_custom;
    }
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_INT)
    in flat int v_custom;

    int getCustom()
    {
        return v_custom;
    }
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_UINT)
    in flat uint v_custom;

    uint getCustom()
    {
        return v_custom;
    }
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_VEC)
    in vec3 v_custom;

    vec3 getCustom()
    {
        return v_custom;
    }
#endif

out vec4 fragColor;

// The order of these outputs must correspond to the order of utils::FramebufferAttachmentTypes
#if defined(ATTRIBUTE_OUTPUT_NORMAL)
    out vec4 o_normal;
#endif

#if defined(ATTRIBUTE_OUTPUT_POSITION)
    out vec4 o_position;
#endif

#if defined(ATTRIBUTE_OUTPUT_SEGMENT)
    out uint o_segment;
#endif

#if defined(ATTRIBUTE_OUTPUT_ID)
    out uint o_id;
#endif

#if defined(ATTRIBUTE_OUTPUT_CUSTOM_FLOAT)
    out float o_custom;
#endif

#if defined(ATTRIBUTE_OUTPUT_CUSTOM_INT)
    out int o_custom;
#endif

#if defined(ATTRIBUTE_OUTPUT_CUSTOM_UINT)
    out uint o_custom;
#endif

#if defined(ATTRIBUTE_OUTPUT_CUSTOM_VEC)
    out vec3 o_custom;
#endif

float getDepth()
{
    return gl_FragCoord.z * 2.0 - 1.0;
}

#COLORIZATION

void main(void)
{
    vec4 color = colorization();
    if (color.a == 0)
        discard;

    fragColor = color;

    #if defined(ATTRIBUTE_OUTPUT_POSITION)
        o_position = vec4(v_position, 1.0);
    #endif

    #if defined(ATTRIBUTE_OUTPUT_NORMAL)
        o_normal = vec4(v_normal, 1.0);
    #endif

    #if defined(ATTRIBUTE_OUTPUT_ID)
        o_id = v_id;
    #endif

    #if defined(ATTRIBUTE_OUTPUT_SEGMENT)
        o_segment = v_segment;
    #endif

    #if defined(ATTRIBUTE_OUTPUT_CUSTOM_FLOAT) || defined(ATTRIBUTE_OUTPUT_CUSTOM_INT) || defined(ATTRIBUTE_OUTPUT_CUSTOM_UINT) || defined(ATTRIBUTE_OUTPUT_CUSTOM_VEC)
        o_custom = v_custom;
    #endif

}