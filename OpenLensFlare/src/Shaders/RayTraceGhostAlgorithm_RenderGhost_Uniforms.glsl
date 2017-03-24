// Lens uniforms
uniform vec3 vLensCenter[64];
uniform vec3 vLensIor[64];
uniform float fLensRadius[64];
uniform float fLensHeight[64];
uniform float fLensAperture[64];
uniform float fLensCoating[64];

// Uniforms
uniform ivec2 vGhostIndices;
uniform int iLength;
uniform int iRayCount;
uniform vec2 vGridCenter;
uniform vec2 vGridSize;
uniform vec2 vImageCenter;
uniform vec2 vImageSize;
uniform vec3 vRayDir;
uniform float fRayDistance;
uniform vec2 vFilmSize;
uniform float fLambda;
uniform float fIntensityScale;
uniform vec4 vColor;
uniform int iRenderMode;
uniform int iShadingMode;
uniform float fRadiusClip;
uniform float fIrisClip;
uniform sampler2D sAperture;