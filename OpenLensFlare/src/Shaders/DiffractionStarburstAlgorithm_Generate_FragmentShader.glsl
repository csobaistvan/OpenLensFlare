// Uniforms
uniform float fMinLambda;
uniform float fMaxLambda;
uniform float fLambdaStep;
uniform float fTextureLambda;
uniform float fApertureDistance;

// Various texture maps
uniform sampler2D sApertureFT;

// Input attribs
in vec2 vUv;

// Render targets
out vec4 colorBuffer;

void main()
{
    // Accumulate the result
    vec3 result_xyz = vec3(0.0);
    for (float lambda = fMinLambda; lambda <= fMaxLambda; lambda += fLambdaStep)
    {
        // Compute the scaling factor for the current wavelength
        float scale = (fTextureLambda / lambda);
        vec2 uv = vUv * scale * 0.5 + 0.5;
        
        // Sample the texture and accumulate the value
        result_xyz += texture(sApertureFT, uv).rgb * lambda2XYZ(lambda, 1.0);
    }
    
    // Convert it to RGB
    vec3 result_rgb = xyz2RGB(result_xyz);

    // Save the result
    colorBuffer = vec4(result_rgb, 1.0);
}
