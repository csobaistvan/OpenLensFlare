#version 330

/// Color of the line strip.
uniform vec4 vLineColor;

/// Output color.
out vec4 outColor;

void main()
{
    outColor = vLineColor;
}