// Uniforms
uniform vec2 vPosition;
uniform vec2 vScale;
uniform vec3 vColor;
uniform vec2 vChannelPosition[];
uniform vec2 vChannelScale[];
uniform vec3 vChannelColor[];
uniform int iChannelCount;

// Various texture maps
uniform sampler2D sAperture;

// Input attribs
in vec2 vUv;

// Render targets
out vec4 colorBuffer;

void main()
{
    // Compute the channel masks
    vec3 outColor = vColor;
    for (int i = 0; i < iChannelCount; ++i)
    {
        // Compute the UV coordinates for the channel
        vec2 channelUV = vUv * vChannelScale[i] + vChannelPosition[i];

        // Compute the distance for the channel
        float channelDist = texture(sAperture, channelUV).r;
        
        // Clamp the UV coordinates with 'step' and apply the mask
        outColor += step(0, channelUV.x) * step(channelUV.y, 1.0) * 
            smoothstep(0.95, 0.8, channelDist) * vChannelColor[i] * vColor;
    }
    
    // Write out the computed color
    colorBuffer.rgb = outColor;
    colorBuffer.a = 1.0;
}