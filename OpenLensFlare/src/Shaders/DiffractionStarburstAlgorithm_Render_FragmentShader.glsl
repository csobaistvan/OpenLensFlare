// Uniforms.
uniform vec2 vPosition;
uniform vec2 vScale;
uniform vec3 vColor;

// Various texture maps
uniform sampler2D sStarburst;

// Input attribs
in vec2 vUv;

// Render targets
out vec4 colorBuffer;

void main()
{
    // Distance of the pixel to the center
    float dist = min(length(vUv * 2.0 - 1.0), 1.0);
    
    // Luminance scaling based on the distance
    float scale = 1.0 - dist;
    scale = 1.0;
    
    // Sample the starburst texture and apply the color and scale factor to it
    colorBuffer.rgb = texture(sStarburst, vUv).rgb * vColor * scale;
    colorBuffer.a = 0.0;
}