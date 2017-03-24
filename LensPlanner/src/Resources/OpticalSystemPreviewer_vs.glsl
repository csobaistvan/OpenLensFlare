#version 330

/// Uniforms.
uniform mat4 mTransform;

/// Vertex attributes.
layout (location = 0) in vec2 vPos;

void main()
{
    // Transform the vertex to apply any scaling, translation, etc.
    gl_Position = mTransform * vec4(vPos, 0, 1);
}