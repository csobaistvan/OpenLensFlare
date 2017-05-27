#include "RayTraceGhostAlgorithm.h"
#include "GLHelpers.h"

#include "Common_Functions.glsl.h"
#include "Common_ColorSpace.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_Uniforms.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_VertexShader.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_GeometryShader.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_FragmentShader.glsl.h"

namespace OLEF
{

/// Standard 3-color wavelengths
static const std::vector<float> STANDARD_WAVELENGTHS = { 650.0f, 510.0f, 475.0f };

////////////////////////////////////////////////////////////////////////////////
RayTraceGhostAlgorithm::RayTraceGhostAlgorithm(OpticalSystem* system):
    m_opticalSystem(system),
	m_lambdas(STANDARD_WAVELENGTHS),
    m_intensityScale(100.0f),
    m_renderMode(RenderMode::PROJECTED_GHOST),
    m_shadingMode(ShadingMode::SHADED),
    m_radiusClip(1.0f),
    m_distanceClip(0.95f),
	m_intensityClip(1.0f),
    m_vao(0)
{
    // Create the precomputation shader
    GLHelpers::ShaderSource precomputeSource;
	
	precomputeSource.m_source =
    {
        {
            GL_VERTEX_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
				Shaders::RayTraceGhostAlgorithm_RenderGhost_Uniforms,
                Shaders::RayTraceGhostAlgorithm_RenderGhost_VertexShader,
            }
        },
        {
            GL_GEOMETRY_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
				Shaders::RayTraceGhostAlgorithm_RenderGhost_Uniforms,
                Shaders::RayTraceGhostAlgorithm_RenderGhost_GeometryShader,
            }
        },
        {
            GL_FRAGMENT_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
				Shaders::RayTraceGhostAlgorithm_RenderGhost_Uniforms,
                Shaders::RayTraceGhostAlgorithm_RenderGhost_FragmentShader,
            }
        },
    };
	precomputeSource.m_defines =
	{
		"#define PRECOMPUTATION 1",
	};
	precomputeSource.m_varyings =
	{
		"vParamGS",
		"vPositionGS",
		"vUvGS",
		"fRadiusGS",
		"fIntensityGS",
		"fIrisDistanceGS"
	};
    m_precomputeShader = GLHelpers::createShader(precomputeSource);

    // Create the render shader
    GLHelpers::ShaderSource renderSource;
	
	renderSource.m_source =
    {
        {
            GL_VERTEX_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
				Shaders::RayTraceGhostAlgorithm_RenderGhost_Uniforms,
                Shaders::RayTraceGhostAlgorithm_RenderGhost_VertexShader,
            }
        },
        {
            GL_GEOMETRY_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
				Shaders::RayTraceGhostAlgorithm_RenderGhost_Uniforms,
                Shaders::RayTraceGhostAlgorithm_RenderGhost_GeometryShader,
            }
        },
        {
            GL_FRAGMENT_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
				Shaders::RayTraceGhostAlgorithm_RenderGhost_Uniforms,
                Shaders::RayTraceGhostAlgorithm_RenderGhost_FragmentShader,
            }
        },
    };
    m_renderShader = GLHelpers::createShader(renderSource);

    // Generate a dummy vertex array.
    glGenVertexArrays(1, &m_vao);
}

RayTraceGhostAlgorithm::~RayTraceGhostAlgorithm()
{
    // Generate a dummy vertex array.
    glDeleteVertexArrays(1, &m_vao);
	
    // Release the shaders
    glDeleteProgram(m_precomputeShader);
    glDeleteProgram(m_renderShader);
}

////////////////////////////////////////////////////////////////////////////////
struct PerVertexData
{
	// Position of the ray on the pupil.
	glm::vec2 m_parameter;

	// Position of the ray's projection on the sensor.
	glm::vec2 m_position;

	// UV coordinates of the ray's hit on the iris.
	glm::vec2 m_uv;

	// Distance of the trace hit from the optical axis.
	GLfloat m_radius;

	// Transmitted light intensity of the ghost.
	GLfloat m_intensity;

	// Distance of the ray to the center of the iris.
	GLfloat m_irisDistance;
};

GhostList RayTraceGhostAlgorithm::computeGhostAttributes(
	const GhostList& ghosts, const GhostAttribComputeParams& computeParams)
{
	// Make a local copy of the original ghost list that we are going to modify
	auto result = ghosts;

    // Find the aperture mask texture
    GLuint apertureTexture = 0;
    for (const auto& lens: m_opticalSystem->getElements())
    {
        if (lens.getType() == OpticalSystemElement::ElementType::APERTURE_STOP)
        {
            apertureTexture = lens.getTexture();
            break;
        }
    }

	// Create the render parameters object
	RenderParameters parameters;

	parameters.m_lightSource.setScreenPosition(glm::vec2(0.0f));
	parameters.m_lightSource.setIncidenceDirection(glm::vec3(
		-glm::sin(computeParams.m_angle), 0.0f, glm::cos(computeParams.m_angle)));
	parameters.m_lightSource.setDiffuseColor(glm::vec3(1.0f));
	parameters.m_lightSource.setDiffuseIntensity(1.0f);

	parameters.m_mask = apertureTexture;
	parameters.m_intensityScale = 1.0f;
	parameters.m_renderMode = RenderMode::PROJECTED_GHOST;
	parameters.m_shadingMode = ShadingMode::SHADED;
	parameters.m_radiusClip = 100000.0f;
	parameters.m_distanceClip = 100000.0f;
	
	// Highest ray grid size that is used while bounding
	int maxRays = *std::max_element(
		computeParams.m_boundingRays.begin(), computeParams.m_boundingRays.end());

	// Create and initialize the read-back buffer// Compute the required buffer sizes
	GLuint readBackBuffer;
	glGenBuffers(1, &readBackBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, readBackBuffer);
	glBufferData(GL_ARRAY_BUFFER, /*TODO: compute this*/0, nullptr, GL_STATIC_READ);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Bind the precomputation shader
	glUseProgram(m_precomputeShader);

	// Disable rasterization
	glEnable(GL_RASTERIZER_DISCARD);

	// Render the selected ghosts
    glBindVertexArray(m_vao);
	for (int chId = 0; chId < STANDARD_WAVELENGTHS.size(); ++chId)
	{
		parameters.m_lambda = STANDARD_WAVELENGTHS[chId];
		for (const auto& ghost: ghosts)
		{
			if (m_opticalSystem->isValidGhost(ghost))
			{
				parameters.m_ghost = ghost;
				//glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, readBackBuffer, byteOffsets[i][0] + ch * byteOffsets[i][1], byteOffsets[i][1]);
				glBeginTransformFeedback(GL_TRIANGLES);
				//renderGhostChannel(parameters);
				glEndTransformFeedback();
			}
		}
	}
    glBindVertexArray(0);

	// Re-enable rasterization
	glDisable(GL_RASTERIZER_DISCARD);

	// Read back the values

	// Process the generated ray data

	// Release the vertex buffer
	glDeleteBuffers(1, &readBackBuffer);

	// Return the refreshed ghost list
	return result;
}

////////////////////////////////////////////////////////////////////////////////
void RayTraceGhostAlgorithm::renderGhostChannel(const RenderParameters& parameters)
{	
	// Calculate the entrance plane's distance from the sensor plane 
	float sensorDistance = m_opticalSystem->getSensorDistance();
		
	// Convert it to spherical angles
	glm::vec3 toLight = -parameters.m_lightSource.getIncidenceDirection();
	float rotation = glm::atan(toLight.y, toLight.x);
	float angle = glm::acos(glm::dot(toLight, glm::vec3(0.0f, 0.0f, -1.0f)));

	// Lambertian shading term
	float lambert = glm::max(glm::dot(toLight, glm::vec3(0.0f, 0.0f, -1.0f)), 0.0f);
	
	// Compute the effective aperture length
	float apertureHeight = m_opticalSystem->getEffectiveApertureHeight();

    // Bind the aperture texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, parameters.m_mask);
	GLHelpers::uploadUniform(m_renderShader, "sAperture", 0);

	// Temporary vectors for the lens parameters
	auto elementCount = m_opticalSystem->getElementCount() + 1;

	/// TODO: these could just be statically sized local arrays on the stack
	std::vector<float> heights(elementCount, 0.0f);
	std::vector<float> curvatures(elementCount, 0.0f);
	std::vector<float> apertures(elementCount, 0.0f);
	std::vector<float>  thicknesses(elementCount, 0.0f);
	std::vector<glm::vec3> centers(elementCount, glm::vec3(0.0f));
	std::vector<glm::vec3> refractions(elementCount, glm::vec3(1.0f));

	// Fill the lens parameter arrays
	float lensDistance = sensorDistance;
	for (int lensId = 1; lensId < elementCount; ++lensId)
	{
		// Reference to the current lens
		const auto& lens = (*m_opticalSystem)[lensId - 1];

		// Set its attributes
		curvatures[lensId] = lens.getRadiusOfCurvature();
		heights[lensId] = lens.getHeight();
		apertures[lensId] = 0.0f;
		centers[lensId].x = 0.0f;
		centers[lensId].y = 0.0f;
		centers[lensId].z = lensDistance - lens.getRadiusOfCurvature();
		refractions[lensId].x = refractions[lensId - 1].z;
		refractions[lensId].y = lens.getCoatingLambda();
		refractions[lensId].z = lens.computeIndexOfRefraction(parameters.m_lambda);
		thicknesses[lensId] = lens.getCoatingLambda() / 4.0f / glm::max(
			glm::sqrt(refractions[lensId][0] * refractions[lensId][2]), 
			refractions[lensId][1]);

		// Special treatment for the special elements
		if (lens.getType() == OpticalSystemElement::ElementType::APERTURE_STOP)
		{
			curvatures[lensId] = 0.0f;
			heights[lensId] = apertureHeight;
			apertures[lensId] = apertureHeight;
		}
		else if (lens.getType() == OpticalSystemElement::ElementType::SENSOR)
		{
			curvatures[lensId] = 0.0f;
			heights[lensId] = glm::min(m_opticalSystem->getFilmWidth(), 
				m_opticalSystem->getFilmHeight());
			apertures[lensId] = 0.0f;
		}

		// The next element is closer
		lensDistance -= lens.getThickness();
	}

	// Compute the remaining attributes
	glm::mat4 rotMat = glm::rotate(rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec3 baseDir = glm::vec3(glm::sin(angle), 0.0f, -glm::cos(angle));

	// Ghost interface indices (increment by one because of the empty
	// space before the front element)
	glm::ivec2 ghostIndices = glm::ivec2(
		parameters.m_ghost[0] + 1, 
		parameters.m_ghost[1] + 1);
	
	// Number of interfaces (including air before)
	GLint elementLength = (GLint) elementCount;
	
	// Ray grid dimensions
	GLint rayCount = parameters.m_ghost.getMinimumRays();
	
	// Direction of the ray
	glm::vec3 rayDir = glm::vec3(rotMat * glm::vec4(baseDir, 1.0f));
	
	// Center of the ray grid
	glm::vec2 gridCenter = glm::mat2(rotMat) * (
		parameters.m_ghost.getPupilBounds()[0] + 
		parameters.m_ghost.getPupilBounds()[1] / 2.0f);
	
	// Size of the ray grid
	glm::vec2 gridSize = parameters.m_ghost.getPupilBounds()[1] / 2.0f;
	
	// Center of the ghost image
	glm::vec2 imageCenter = glm::mat2(rotMat) * (
		parameters.m_ghost.getSensorBounds()[0] + 
		parameters.m_ghost.getSensorBounds()[1] / 2.0f);
	
	// Size of the ghost image
	glm::vec2 imageSize = parameters.m_ghost.getSensorBounds()[1] / 2.0f;
	
	// Distance of the ray from the sensor, along the optical axis
	GLfloat rayDist = sensorDistance + 0.1f;
	
	// Size of the film
	glm::vec2 filmSize = m_opticalSystem->getFilmSize();

	// Wavelength at which we render
	float lambda = parameters.m_lambda;

	// Lambertian coefficient
	GLfloat intensity = lambert * parameters.m_intensityScale;

	// Light color
	glm::vec4 color = glm::vec4(parameters.m_lightSource.getDiffuseColor() * 
		parameters.m_lightSource.getDiffuseIntensity(),
		1.0f);

	// Ghost rendering mode (ghost, ray grid, etc.)
	GLint renderMode = (GLint) parameters.m_renderMode;

	// Ghost shading mode (shaded, unlit, UV, etc.)
	GLint shadingMode = (GLint) parameters.m_shadingMode;

	// Radius clipping
	GLfloat radiusClip = parameters.m_radiusClip;

	// Iris clipping.
	GLfloat irisClip = parameters.m_distanceClip;

	// Upload all the uniforms
	GLHelpers::uploadUniform(m_renderShader, "vLensCenter", centers);
	GLHelpers::uploadUniform(m_renderShader, "vLensIor", refractions);
	GLHelpers::uploadUniform(m_renderShader, "fLensRadius", curvatures);
	GLHelpers::uploadUniform(m_renderShader, "fLensHeight", heights);
	GLHelpers::uploadUniform(m_renderShader, "fLensAperture", apertures);
	GLHelpers::uploadUniform(m_renderShader, "fLensCoating", thicknesses);

	GLHelpers::uploadUniform(m_renderShader, "vGhostIndices", ghostIndices);
	GLHelpers::uploadUniform(m_renderShader, "iLength", elementLength);
	GLHelpers::uploadUniform(m_renderShader, "iRayCount", rayCount);
	GLHelpers::uploadUniform(m_renderShader, "vRayDir", rayDir);
	GLHelpers::uploadUniform(m_renderShader, "vGridCenter", gridCenter);
	GLHelpers::uploadUniform(m_renderShader, "vGridSize", gridSize);
	GLHelpers::uploadUniform(m_renderShader, "vImageCenter", imageCenter);
	GLHelpers::uploadUniform(m_renderShader, "vImageSize", imageSize);
	GLHelpers::uploadUniform(m_renderShader, "fRayDistance", rayDist);
	GLHelpers::uploadUniform(m_renderShader, "vFilmSize", filmSize);
	GLHelpers::uploadUniform(m_renderShader, "fLambda", lambda);
	GLHelpers::uploadUniform(m_renderShader, "fIntensityScale", intensity);
	GLHelpers::uploadUniform(m_renderShader, "vColor", color);
	GLHelpers::uploadUniform(m_renderShader, "iRenderMode", renderMode);
	GLHelpers::uploadUniform(m_renderShader, "iShadingMode", shadingMode);
	GLHelpers::uploadUniform(m_renderShader, "fRadiusClip", radiusClip);
	GLHelpers::uploadUniform(m_renderShader, "fIrisClip", irisClip);

	// Render the tessellated quad
	int vertexCount = (rayCount - 1) * (rayCount - 1) * 6;
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

////////////////////////////////////////////////////////////////////////////////
void RayTraceGhostAlgorithm::renderGhosts(const LightSource& light, const GhostList& ghosts)
{
    // Find the aperture mask texture
    GLuint apertureTexture = 0;
    for (const auto& lens: m_opticalSystem->getElements())
    {
        if (lens.getType() == OpticalSystemElement::ElementType::APERTURE_STOP)
        {
            apertureTexture = lens.getTexture();
            break;
        }
    }

	// Create the render parameters object
	RenderParameters parameters;

	parameters.m_lightSource = light;
	parameters.m_mask = apertureTexture;
	parameters.m_intensityScale = m_intensityScale;
	parameters.m_renderMode = m_renderMode;
	parameters.m_shadingMode = m_shadingMode;
	parameters.m_radiusClip = m_radiusClip;
	parameters.m_distanceClip = m_distanceClip;
	
	// Bind the render shader
	glUseProgram(m_renderShader);

	// Render the selected ghosts
    glBindVertexArray(m_vao);
	for (const auto& ghost: ghosts)
	{
		if (m_opticalSystem->isValidGhost(ghost) && 
			ghost.getAverageIntensity() >= m_intensityClip)
		{
			parameters.m_ghost = ghost;
			for (int ch = 0; ch < ghost.getMinimumChannels(); ++ch)
			{
				parameters.m_lambda = m_lambdas[ch];
				renderGhostChannel(parameters);
			}
		}
	}
    glBindVertexArray(0);
}

}