#version 450

uniform int u_PointSize;
uniform ivec2 u_FramebufferSize;

layout (std430, binding=0) buffer inputDepthBuffer
{
    uint depthBuffer[];
};

layout (std430, binding=1) buffer inputRGBABuffer
{
    uint rgbaBuffer[];
};

out vec4 fragColor;
out float gl_FragDepth;

void main(void)
{
    uint r = 0;
    uint g = 0;
    uint b = 0;
    uint a = 0;
    float depth = 1.0f;

    int pixel_x = int(gl_FragCoord.x - u_PointSize/2);
    int pixel_y = int(gl_FragCoord.y - u_PointSize/2);

    for(int i = 0; i < u_PointSize * u_PointSize; i++) {
        int new_x = int(clamp(pixel_x + i / u_PointSize, 0, u_FramebufferSize.x-1));
        int new_y = int(clamp(pixel_y + i % u_PointSize, 0, u_FramebufferSize.y-1));

        int index = new_x + new_y * u_FramebufferSize.x;

        float new_depth = uintBitsToFloat(depthBuffer[index]);
        if(new_depth < depth) {
            depth = new_depth;
            r = rgbaBuffer[4*index+0];
            g = rgbaBuffer[4*index+1];
            b = rgbaBuffer[4*index+2];
            a = rgbaBuffer[4*index+3];
        }
    }

    if (a == 0)
        discard;

    vec4 color = vec4(float(r / a) / 255.0f, float(g / a) / 255.0f, float(b / a) / 255.0f, 1.0f);

    fragColor = color;
    gl_FragDepth = depth;
}