#version 460 core

#SHADER_DEFINES // Here the VERTEX_SHADER_OUTPUT_XYZ get placed by the ShaderProgramFactory before compilation of the shader

uniform mat4 u_mvp;
uniform float u_point_size = 1.0;

struct Node
{   vec3 bbox_min;
    float is_leaf;
    vec3 bbox_max;
    uint parent_index_and_render_flag;
};

layout(std430, binding=0) readonly restrict buffer NodesBuffer
{
    Node node_metadata[];
};


layout(std430, binding=1) readonly restrict buffer DrawMetadataBuffer
{
    uint positions_buffer_offset[];
};

layout(std430, binding=2) readonly restrict buffer PositionBuffer {
    float a_position[];
};

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
    // we store the node index in gl_BaseInstance
    Node node = node_metadata[gl_BaseInstance];
    vec3 position;
    if(node.is_leaf > 0.0f)
    {
        uint offset = gl_VertexID * 3;  // = buffer_offset * 3 + (gl_VertexID - buffer_offset) * 3
        position.x = a_position[offset];
        position.y = a_position[offset + 1];
        position.z = a_position[offset + 2];
   }
    else
    {
        uint offset = positions_buffer_offset[gl_DrawID] * 2 + gl_VertexID;  // = buffer_offset * 3 + (gl_VertexID - buffer_offset)
        uint packed_pos = floatBitsToUint(a_position[offset]);
        position.x = float((packed_pos) & 0xFFu);
        position.y = float((packed_pos >> 8) & 0xFFu);
        position.z = float((packed_pos >> 16) & 0xFFu);
        position += vec3(0.5f);
        position = (position / 128.0f) * (node.bbox_max - node.bbox_min) + node.bbox_min;
    }

    gl_Position = u_mvp * vec4(position, 1.0);
    //gl_PointSize = u_point_size * pow(1.1f, float(gl_BaseInstance));
    gl_PointSize = u_point_size;

    #if defined(ATTRIBUTE_INPUT_POSITION)
        v_position = position;
    #endif

    #if defined(ATTRIBUTE_INPUT_COLOR)
        v_color = a_color;

        // per-level coloring
        /*
        uint level = 0u;
        uint parent_index = node_metadata[gl_BaseInstance].parent_index_and_render_flag & 0x7FFFFFFFu;
        while (parent_index != 0x7FFFFFFFu)
        {
            parent_index = node_metadata[parent_index].parent_index_and_render_flag & 0x7FFFFFFFu;
            level++;
        }

        uint hashed = (level + 3u) * 2654435761u;
        v_color = vec3(float(hashed & 255u) / 255.0, float((hashed >> 8) & 255u) / 255.0, float((hashed >> 16) & 255u) / 255.0);
        */

        // per-node coloring
        //uint hashed = (uint(gl_BaseInstance) + 1u) * 2654435761u;
	    //v_color = vec3(float(hashed & 255u) / 255.0, float((hashed >> 8) & 255u) / 255.0, float((hashed >> 16) & 255u) / 255.0);

        // within-node point order coloring
        //uint hashed = (uint(gl_BaseInstance) + 1u) * 2654435761u;
        //v_color = vec3(float(hashed & 255u) / 255.0, float((hashed >> 8) & 255u) / 255.0, float((hashed >> 16) & 255u) / 255.0);
        //v_color *= 0.5 + 0.5 * float(gl_VertexID - positions_buffer_offset[gl_DrawID]) / 50000.0f;
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
