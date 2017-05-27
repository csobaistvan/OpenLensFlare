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

// Render modes
#define RENDER_MODE_PROJECTED_GHOST 0
#define RENDER_MODE_PUPIL_GRID      1

// Shading modes
#define SHADING_MODE_SHADED          0
#define SHADING_MODE_UNCOLORED       1
#define SHADING_MODE_UNSHADED        2
#define SHADING_MODE_RAY_COORDINATES 3
#define SHADING_MODE_UV_COORDINATES  4
#define SHADING_MODE_RELATIVE_RADIUS 5