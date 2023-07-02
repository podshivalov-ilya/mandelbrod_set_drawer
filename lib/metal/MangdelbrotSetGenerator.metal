#include <metal_stdlib>

using namespace metal;

float4 coldColorMap(float value)
{
    return float4(min(3.0 * value, 1.0),
                 min(3.0 * value - 1.0, 1.0),
                 min(3.0 * value - 2.0, 1.0),
                 1.0);
}

float4 hotColorMap(float value)
{
    return float4(min(3.0 * value - 2.0, 1.0),
                 min(3.0 * value - 1.0, 1.0),
                 min(3.0 * value, 1.0),
                 1.0);
}

float4 greenColorMap(float value)
{
    return float4(min(3.0 * value - 1.0, 1.0),
                 min(3.0 * value, 1.0),
                 min(3.0 * value - 2.0, 1.0),
                 1.0);
}

float4 simpleColorMap(float value)
{
    return float4(value, value, value, 1.0);
}

float3 hsvToRgb(float3 hsv)
{
    float h60 = hsv.x / 60.0;
    int hmod6 = int(h60) % 6;
    float f = h60 - floor(h60);
    float3 pqt(hsv.z * (1 - hsv.y),
               hsv.z * (1 - f * hsv.y),
               hsv.z * (1 - (1 - f) * hsv.y));
    float3 rgb(0.0, 0.0, 0.0);
    if (hmod6 == 0)
        rgb = float3(hsv.z, pqt.z, pqt.x); // V T P
    else if (hmod6 == 1)
        rgb = float3(pqt.y, hsv.z, pqt.x); // Q V P
    else if (hmod6 == 2)
        rgb = float3(pqt.x, hsv.z, pqt.z); // P V T
    else if (hmod6 == 3)
        rgb = float3(pqt.x, pqt.y, hsv.z); // P Q V
    else if (hmod6 == 4)
        rgb = float3(pqt.z, pqt.x, hsv.z); // T P V
    else if (hmod6 == 5)
        rgb = float3(hsv.z, pqt.x, pqt.y); // V P Q
    return rgb;
}

float4 rainbowColorMap(float value)
{
    float hue = value * 360.0;
    float3 rgb = hsvToRgb(float3(hue, 1.0, 1.0));
    return float4(rgb.x, rgb.y, rgb.z, 1.0);
}

float4 cubicInterpolation(float value, float4 c1, float4 c2)
{
    float coef = (1.0 - pow((1.0 - value), 3.0));
    return c1 + (c2 - c1) * coef;
}

float4 cubicInterpolatedColorMap(float value)
{
    float4 red(1.0, 0.0, 0.0, 0.0);
    float4 green(0.0, 1.0, 0.0, 0.0);
    float4 blue(0.0, 0.0, 1.0, 0.0);
    if (value < 0.5)
        return cubicInterpolation(2.0 * value, red, green);
    //else if (value > 0.33 && value < 0.66)
    //    return cubicInterpolation(3.0 * (value - 0.33), green, blue);
    return cubicInterpolation(2.0 * (value - 0.5), green, blue);

}

uint floatColorToInt(float value) {
    return uint(clamp(value, 0.0, 1.0) * 255.0);
}

kernel void mandelbrot(texture2d<uint, access::write> image [[texture(0)]],
                       device float3 *position [[buffer(0)]],
                       device uint64_t *maxIterations [[buffer(1)]],
                       uint2 index [[thread_position_in_grid]],
                       uint2 gridSize [[threads_per_grid]])
{
    const float scale(position->z);
    const float2 center(position->x, position->y);
    const float width = gridSize.x;
    const float height = gridSize.y;
    const float x = index.x;
    const float y = index.y;

    const float2 c = float2(scale * (x - width / 2.0) / width + center.x,
                            scale * (y - height / 2.0) / height + center.y);

    float2 z = c;
    uint64_t i;
    for (i = 0; i < *maxIterations; ++i) {
        float2 zSquared = float2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y);
        z = zSquared + c;
        if (length(z) > 2.0) {
            break;
        }
    }
    float colorfulValue = *maxIterations;
    float lengthZ = length(z);
    if (lengthZ > 1 && i < *maxIterations)
        colorfulValue = i + 1.0 - log(log2(lengthZ));
    colorfulValue /= *maxIterations; // normalization;
    float4 pixel = rainbowColorMap(colorfulValue);
    uint4 px = uint4(clamp(pixel, 0.0, 1.0) * 255.0);
    image.write(px, index);
}
