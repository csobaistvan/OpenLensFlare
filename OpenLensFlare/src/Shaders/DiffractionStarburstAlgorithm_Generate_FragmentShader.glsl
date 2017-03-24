// Uniforms
uniform float fMinLambda;
uniform float fMaxLambda;
uniform float fLambdaStep;
uniform float fTextureLambda;

// Various texture maps
uniform sampler2D sApertureFT;

// Input attribs
in vec2 vUv;

// Render targets
out vec4 colorBuffer;

//void main()
//{    
//    // Accumulate the result
//    vec3 result = vec3(0.0);
//    for (float lambda = fMinLambda; lambda <= fMaxLambda; lambda += fLambdaStep)
//    {
//        // Compute the scaling factor for the current wavelength
//        float scale = (fTextureLambda / lambda);
//        vec2 uv = clamp(vUv * scale, vec2(-1.0), vec2(1.0)) * 0.5 + 0.5;
//        
//        // Sample the texture and accumulate the value
//        result += texture(sApertureFT, uv).rgb * lambda2RGB(lambda, 1.0);
//    }
//    
//    // Save the result
//    colorBuffer = vec4(result, 1.0);
//}

void main()
{
    // Accumulate the result
    vec3 result_xyz = vec3(0.0);
    for (float lambda = fMinLambda; lambda <= fMaxLambda; lambda += fLambdaStep)
    {
        // Compute the scaling factor for the current wavelength
        float scale = (fTextureLambda / lambda);
        vec2 uv = clamp(vUv * scale, vec2(-1.0), vec2(1.0)) * 0.5 + 0.5;
        
        // Sample the texture and accumulate the value
        result_xyz += texture(sApertureFT, uv).rgb * lambda2XYZ(lambda, 1.0);
    }
    
    // Convert it to RGB
    vec3 result_rgb = xyz2RGB(XYZ2xyz(result_xyz));

    // Save the result
    colorBuffer = vec4(result_rgb, 1.0);
}