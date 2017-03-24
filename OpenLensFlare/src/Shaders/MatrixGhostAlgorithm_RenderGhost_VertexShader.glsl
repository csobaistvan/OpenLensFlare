// Uniforms
uniform vec2 vPosition;
uniform vec2 vScale;
uniform vec3 vColor;
uniform vec2 vChannelPosition[];
uniform vec2 vChannelScale[];
uniform vec3 vChannelColor[];
uniform int iChannelCount;

// Output attribs
out vec2 vUv;

// Quad vertices
vec2 POSITIONS[6] = vec2[6]
(
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0),
    
    vec2(-1.0, -1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0)
);

// Entry point
void main()
{
    // Compute the UV coordinate
    vUv = POSITIONS[gl_VertexID] * 0.5 + 0.5;
    
    // Compute the vertex position
    gl_Position = vec4(vPosition + vScale * POSITIONS[gl_VertexID], 0, 1);
}