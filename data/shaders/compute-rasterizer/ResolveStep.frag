#version 450 core
#extension GL_ARB_gpu_shader_int64 : require

uniform int u_PointSize;
uniform ivec2 u_FramebufferSize;

layout (std430, binding=0) buffer renderBuffer
{
    uint64_t data[];
};

struct vector3
{
    float x;
    float y;
    float z;
};

uint id = 0;

float depth = 1.0f;
float getDepth()
{
    return depth;
}

#SHADER_DEFINES // Here the VERTEX_SHADER_OUTPUT_XYZ get placed by the ShaderProgramFactory before compilation of the shader

#if defined(ATTRIBUTE_INPUT_POSITION)
    layout (std430, binding=1) readonly buffer inputPosition
    {
        vector3 sourcePosition[];
    };

    vec3 getPosition()
    {
        uint offset = id;
        return vec3(sourcePosition[offset].x, sourcePosition[offset].y, sourcePosition[offset].z);
    }
#endif

#if defined(ATTRIBUTE_INPUT_COLOR)
    layout (std430, binding=2) readonly buffer inputColor
    {
        uint sourceColor[];
    };

    vec3 getColor()
    {
        uint base_offset = id * 3;

        uint offset_r = base_offset >> 2;     // >> 2 corresponds to / 4 but is potentially faster
        uint offset_g = (base_offset + 1) >> 2;
        uint offset_b = (base_offset + 2) >> 2;

        uint shift_r = base_offset & 3;    // &3 corresponds to %4 but is potentially faster;
        uint shift_g = (base_offset + 1) & 3;
        uint shift_b = (base_offset + 2) & 3;

        float r = float((sourceColor[offset_r] >> 8 * shift_r) & 0xFFu) / 255.0;
        float g = float((sourceColor[offset_g] >> 8 * shift_g) & 0xFFu) / 255.0;
        float b = float((sourceColor[offset_b] >> 8 * shift_b) & 0xFFu) / 255.0;

        return vec3(r, g, b);
    }
#endif

#if defined(ATTRIBUTE_INPUT_NORMAL)
    layout (std430, binding=3) readonly buffer inputNormal
    {
        vector3 sourceNormal[];
    };

    vec3 getNormal()
    {
        uint offset = id;
        return vec3(sourceNormal[offset].x, sourceNormal[offset].y, sourceNormal[offset].z);
    }
#endif

#if defined(ATTRIBUTE_INPUT_ID)
    layout (std430, binding=4) readonly buffer inputId
    {
        uint sourceId[];
    };

    uint getId()
    {
        uint offset = id;
        return sourceId[offset];
    }
#endif

#if defined(ATTRIBUTE_INPUT_SEGMENT)
    layout (std430, binding=5) readonly buffer inputSegmentId
    {
        uint sourceSegmentId[];
    };

    uint getSegment()
    {
        uint offset = id;
        return inputSegmentId[offset];
    }
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_FLOAT)
    layout (std430, binding=6) readonly buffer inputCustom
    {
        float sourceCustom[];
    };

    float getCustom()
    {
        uint offset = id;
        return sourceCustom[offset];
    }
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_INT)
    layout (std430, binding=6) readonly buffer inputCustom
    {
        int sourceCustom[];
    };

    int getCustom()
    {
        uint offset = id;
        return sourceCustom[offset];
    }
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_UINT)
    layout (std430, binding=6) readonly buffer inputCustom
    {
        uint sourceCustom[];
    };

    uint getCustom()
    {
        uint offset = id;
        return sourceCustom[offset];
    }
#endif

#if defined(ATTRIBUTE_INPUT_CUSTOM_VEC)
    layout (std430, binding=6) readonly buffer inputCustom
    {
        vector3 sourceCustom[];
    };

    vec3 getCustom()
    {
        uint offset = id;
        return vec3(sourceCustom[offset].x, sourceCustom[offset].y, sourceCustom[offset].z);
    }
#endif

#COLORIZATION

uint64_t bitExtract(uint64_t val, uint64_t offset, uint64_t size) {
    uint64_t mask = (uint64_t(1) << size) - 1;
    return uint64_t(val >> offset) & mask;
}

out vec4 fragColor;
out float gl_FragDepth;

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

void main(void)
{
    int pixel_x = int(gl_FragCoord.x - u_PointSize/2);
    int pixel_y = int(gl_FragCoord.y - u_PointSize/2);

    for(int i = 0; i < u_PointSize * u_PointSize; i++) {
        int new_x = clamp(pixel_x + i / u_PointSize, 0, u_FramebufferSize.x-1);
        int new_y = clamp(pixel_y + i % u_PointSize, 0, u_FramebufferSize.y-1);

        uint64_t new_depth_and_point = data[new_x + new_y * uint(u_FramebufferSize.x)];
        float new_depth = intBitsToFloat(int(bitExtract(new_depth_and_point, 32ul, 32ul)));
        if(new_depth < depth) {
            depth = new_depth;
            id = uint(bitExtract(new_depth_and_point, 0ul, 32ul));
        }
    }

    vec4 color = colorization();
    if (color.a == 0)
        discard;

    fragColor = color;
    gl_FragDepth = depth;

    #if defined(ATTRIBUTE_OUTPUT_POSITION)
        o_position = vec4(getPosition(), 1.0);
    #endif

    #if defined(ATTRIBUTE_OUTPUT_NORMAL)
        o_normal = vec4(getNormal(), 1.0);
    #endif

    #if defined(ATTRIBUTE_OUTPUT_ID)
        o_id = getId();
    #endif

    #if defined(ATTRIBUTE_OUTPUT_SEGMENT)
        o_segment = getSegment();
    #endif

    #if defined(ATTRIBUTE_OUTPUT_CUSTOM_FLOAT) || defined(ATTRIBUTE_OUTPUT_CUSTOM_INT) || defined(ATTRIBUTE_OUTPUT_CUSTOM_UINT) || defined(ATTRIBUTE_OUTPUT_CUSTOM_VEC)
        o_custom = getCustom();
    #endif
}