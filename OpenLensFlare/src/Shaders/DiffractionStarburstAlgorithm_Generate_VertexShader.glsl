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
    vUv = POSITIONS[gl_VertexID];
    gl_Position = vec4(POSITIONS[gl_VertexID], 0, 1);
}