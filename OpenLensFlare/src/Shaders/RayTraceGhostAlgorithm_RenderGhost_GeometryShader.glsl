// Inputs are triangles
layout(triangles) in;

// Outputs are also triangles
layout(triangle_strip, max_vertices = 3) out;

// Inputs
in vec2 vParam[];
in vec2 vPos[];
in vec2 vUv[];
in float fRadius[];
in float fIntensity[];

// Outputs
#ifdef GHOST_GENERATION
out float fIrisDistance;
#endif
out vec2 vParamGS;
out vec2 vPositionGS;
out vec2 vUvGS;
out float fRadiusGS;
out float fIntensityGS;
out vec3 vColorGS;

void main()
{    
    // Height of the pupil lens
    float pupilHeight = fLensHeight[1];

    // Calculate the area of the quad on the pupil
    float pupilArea = 
        (vGridSize.x * pupilHeight) * 
        (vGridSize.y * pupilHeight);
    
    // Compute the area of the whole pupil
    float wholePupilArea = pow(2.0 * pupilHeight, 2.0);

    // Compute the area of the image on the sensor
    float sensorArea = vImageSize.x * vImageSize.y;

    // Compute the scaled intensity for the triangle
    float intensity = pupilArea / wholePupilArea / sensorArea;
    
    // Write out the vertices
    for (int i = 0; i < 3; ++i)
    {
        vParamGS = vParam[i];
        vPositionGS = vPos[i];
        vUvGS = vUv[i];
        fRadiusGS = fRadius[i];
        fIntensityGS = clamp(fIntensity[i], 0, 1);
        
        #ifdef GHOST_GENERATION
        vec2 normalizedUv = clamp(vUv[i], vec2(-1.0), vec2(1.0)) * 0.5 + 0.5;
        fIrisDistance = texture(sAperture, normalizedUv).r;
        #endif
        
        vColorGS = lambda2RGB(fLambda, 1.0) * intensity * fIntensityScale;
        gl_Position = vec4(vPositionGS, 0, 1);
        EmitVertex();
    }
        
    EndPrimitive();
}