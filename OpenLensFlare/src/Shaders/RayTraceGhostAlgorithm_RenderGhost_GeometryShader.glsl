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
out vec2 vParamGS;      // Coordinates of the originating ray on the pupil element
out vec2 vUvGS;         // UV coordinates of the ray passing the aperture
out float fRadiusGS;    // Relative radius
out float fIntensityGS; // Transmitted energy factor
out vec3 vColorGS;      // Channel of the color, scaled by the various scaling
                        // factors (but not by fIntensityGS)

// Additional outputs, for readback through transform feedback
#ifdef PRECOMPUTATION
out vec2 vPositionGS;    // Projected ray position on the sensor
out float fIrisDistanceGS; // Iris texture sampled by the UV
#endif

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
    
    // Generate vertices
    for (int i = 0; i < 3; ++i)
    {
        vParamGS = vParam[i];
        vUvGS = vUv[i];
        fRadiusGS = fRadius[i];
        fIntensityGS = clamp(fIntensity[i], 0, 1);
        vColorGS = lambda2RGB(fLambda, 1.0) * intensity * fIntensityScale;
        gl_Position = vec4(vPos[i], 0, 1);
        
        #ifdef PRECOMPUTATION
        vec2 normalizedUv = clamp(vUv[i], vec2(-1.0), vec2(1.0)) * 0.5 + 0.5;
        fIrisDistanceGS = texture(sAperture, normalizedUv).r;
        vPositionGS = vPos[i];
        #endif

        EmitVertex();
    }
        
    EndPrimitive();
}