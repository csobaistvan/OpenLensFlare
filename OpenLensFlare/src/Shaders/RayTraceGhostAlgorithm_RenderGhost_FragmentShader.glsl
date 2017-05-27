// Input values, as explained in the geometry shader
in vec2 vParamGS;
in vec2 vUvGS;
in float fRadiusGS;
in float fIntensityGS;
in vec3 vColorGS;
#ifdef PRECOMPUTATION
in vec2 vPositionGS;
in float fIrisDistanceGS;
#endif

// Framebuffer output value
out vec4 colorBuffer;

void main()
{
    // Apply clipping based on the relative radius
    if (fRadiusGS > fRadiusClip)
        discard;
    
    // Compute the UV coordinates
    vec2 uv = clamp(vUvGS, vec2(-1.0), vec2(1.0)) * 0.5 + 0.5;
    
    // Sample the texture mask
    float mask = 1.0 - step(fIrisClip, texture(sAperture, uv).r);
    
    // Shading mode: shaded
    if (iShadingMode == SHADING_MODE_SHADED)
    {
        // Write out the final color
        colorBuffer = vec4(vColorGS, 1) * vColor * fIntensityGS * mask;
    }
    
    // Shading mode: uncolored
    else if (iShadingMode == SHADING_MODE_UNCOLORED)
    {
        colorBuffer = vec4(1) * fIntensityGS * mask;
    }
    
    // Shading mode: unshaded
    else if (iShadingMode == SHADING_MODE_UNSHADED)
    {
        // Write out only the mask
        colorBuffer = vec4(mask);
    }
    
    // Shading mode: ray coords
    else if (iShadingMode == SHADING_MODE_RAY_COORDINATES)
    {
        colorBuffer = vec4(abs(vParamGS), 0, 1.0) * mask;
    }
    
    // Shading mode: UV coords
    else if (iShadingMode == SHADING_MODE_UV_COORDINATES)
    {
        colorBuffer = vec4(uv, 0, 1.0) * mask;
    }
    
    // Shading mode: relative radius
    else if (iShadingMode == SHADING_MODE_RELATIVE_RADIUS)
    {
        colorBuffer = vec4(vec3(fRadiusGS), 1) * mask;
        
        if (fRadiusGS > 1.0)
            colorBuffer.gb = vec2(0.0);
    }

    else
    {
        colorBuffer = vec4(1, 0, 1, 1);
    }
}