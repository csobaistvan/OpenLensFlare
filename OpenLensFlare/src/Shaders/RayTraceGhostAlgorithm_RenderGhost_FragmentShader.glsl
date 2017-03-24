// Input values
#ifdef GHOST_GENERATION
in vec2 vPositionGS;
in float fIrisDistance;
#endif
in vec2 vParamGS;
in vec2 vUvGS;
in float fRadiusGS;
in float fIntensityGS;
in vec3 vColorGS;

// Framebuffer
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
    if (iShadingMode == 0.0)
    {
        // Write out the final color
        colorBuffer = vec4(vColorGS, 1) * vColor * fIntensityGS * mask;
    }
    
    // Shading mode: unshaded
    else if (iShadingMode == 1.0)
    {
        // Write out only the mask
        colorBuffer = vec4(mask);
    }
    
    // Shading mode: ray coords
    else if (iShadingMode == 2.0)
    {
        colorBuffer = vec4(abs(vParamGS), 0, 1.0) * mask;
        
        // Only render the red channel
        if (fLambda < 600.0f)
            colorBuffer = vec4(0);
    }
    
    // Shading mode: UV coords
    else if (iShadingMode == 3.0)
    {
        colorBuffer = vec4(uv, 0, 1.0) * mask;
        
        // Only render the red channel
        if (fLambda < 600.0f)
            colorBuffer = vec4(0);
    }
    
    // Shading mode: relative radius
    else if (iShadingMode == 4.0)
    {
        colorBuffer = vec4(0.5 + fRadiusGS / 2.0) * mask;
        
        if (fRadiusGS > 1.0)
            colorBuffer.gb = vec2(0.0);
        
        // Only render the red channel
        if (fLambda < 600.0f)
            colorBuffer = vec4(0);
    }
}