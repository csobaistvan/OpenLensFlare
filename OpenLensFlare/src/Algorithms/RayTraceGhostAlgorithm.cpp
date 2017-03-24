#include "RayTraceGhostAlgorithm.h"
#include "GLHelpers.h"

#include "Common_Version.glsl.h"
#include "Common_Functions.glsl.h"
#include "Common_ColorSpace.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_Uniforms.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_VertexShader.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_GeometryShader.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_FragmentShader.glsl.h"

namespace OLEF
{

////////////////////////////////////////////////////////////////////////////////
RayTraceGhostAlgorithm::RayTraceGhostAlgorithm():
    RayTraceGhostAlgorithm(nullptr, GhostList{})
{}

RayTraceGhostAlgorithm::RayTraceGhostAlgorithm(OpticalSystem* system, const GhostList& ghosts):
    m_opticalSystem(system),
    m_ghosts(ghosts),
    m_vao(0),
	m_lambdas({ 650.0f, 510.0f, 475.0f } ),
    m_intensityScale(1.0f),
    m_renderMode(0),
    m_shadeMode(0),
    m_radiusClip(1.0f),
    m_distanceClip(0.95f)
{
    // Create the render shader
    GLHelpers::ShaderSource renderSource =
    {
        {
            GL_VERTEX_SHADER, 
            {
                Shaders::Common_Version,
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
				Shaders::RayTraceGhostAlgorithm_RenderGhost_Uniforms,
                Shaders::RayTraceGhostAlgorithm_RenderGhost_VertexShader,
            }
        },
        {
            GL_GEOMETRY_SHADER, 
            {
                Shaders::Common_Version,
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
				Shaders::RayTraceGhostAlgorithm_RenderGhost_Uniforms,
                Shaders::RayTraceGhostAlgorithm_RenderGhost_GeometryShader,
            }
        },
        {
            GL_FRAGMENT_SHADER, 
            {
                Shaders::Common_Version,
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
	
    // Release the shader
    glDeleteProgram(m_renderShader);
}

////////////////////////////////////////////////////////////////////////////////
void RayTraceGhostAlgorithm::computeGhostAttributes(const ComputeParams& params)
{}

////////////////////////////////////////////////////////////////////////////////
bool RayTraceGhostAlgorithm::serialize(const std::ostream& stream) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool RayTraceGhostAlgorithm::deserialize(const std::istream& stream)
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void RayTraceGhostAlgorithm::renderGhosts(const LightSource& light, 
	int first, int count)
{
    // The aperture texture
    GLuint apertureTexture = 0;
    for (const auto& lens: m_opticalSystem->getElements())
    {
        if (lens.getType() == OpticalSystemElement::ElementType::APERTURE_STOP)
        {
            apertureTexture = lens.getTexture();
            break;
        }
    }
	
	// Calculate the entrance plane's distance from the sensor plane 
	float sensorDistance = m_opticalSystem->getSensorDistance();
		
	// Convert it to spherical angles
	glm::vec3 toLight = -light.getIncidenceDirection();
	float rotation = glm::atan(toLight.y, toLight.x);
	float angle = glm::acos(glm::dot(toLight, glm::vec3(0.0f, 0.0f, -1.0f)));

	// Lambertian shading term
	float lambert = glm::max(glm::dot(toLight, glm::vec3(0.0f, 0.0f, -1.0f)), 0.0f);

	// Extract the required ghost bounding information
	auto ghostAttribsIt = m_attributes.lower_bound(angle - 0.05f);
	float clampedAngle;
	GhostAttributesList ghostAttribs;

	// Make sure we found an appropriate one...
	if (ghostAttribsIt != m_attributes.end() && 
		glm::abs(ghostAttribsIt->first - angle) < 0.75f)
	{
		clampedAngle = ghostAttribsIt->first;
		ghostAttribs = ghostAttribsIt->second;
	}

	// Or fall back to a stock one
	else
	{
		clampedAngle = angle;
		ghostAttribs.resize(count);
	}
	
	// Compute the effective aperture length
	float effectiveApertureHeight = (m_opticalSystem->getEffectiveFocalLength() /
		 m_opticalSystem->getFnumber()) / 2.0f;
	
	// Bind the ghosts shader
	glUseProgram(m_renderShader);

    // Bind the aperture texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, apertureTexture);
	GLHelpers::uploadUniform(m_renderShader, "sAperture", 0);

	// Temporary vectors for the lens parameters
	auto elementCount = m_opticalSystem->getElementCount() + 1;
	std::vector<float> heights(elementCount), curvatures(elementCount), 
		apertures(elementCount), thicknesses(elementCount);
	std::vector<glm::vec3> centers(elementCount), refractions(elementCount);

	// Initialize the element corresponding to the air before the system
	heights[0] = 100000.0f;
	curvatures[0] = 0.0f;
	apertures[0] = 0.0f;
	thicknesses[0] = 0.0f;
	centers[0] = glm::vec3(0.0f);
	refractions[0] = glm::vec3(1.0f);

	// Render the selected ghosts
    glBindVertexArray(m_vao);
	for (int i = first; i < first + count; ++i)
	{
		// Render each channel
		for (int ch = 0; ch < ghostAttribs[i].getMinimumChannels(); ++ch)
		{
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
				refractions[lensId].z = lens.computeIndexOfRefraction(m_lambdas[ch]);
				thicknesses[lensId] = lens.getCoatingLambda() / 4.0f / glm::max(
					glm::sqrt(refractions[lensId][0] * refractions[lensId][2]), 
						refractions[lensId][1]);

				// Special treatment for the special elements
				if (lens.getType() == OpticalSystemElement::ElementType::APERTURE_STOP)
				{
					curvatures[lensId] = 0.0f;
					heights[lensId] = effectiveApertureHeight;
					apertures[lensId] = effectiveApertureHeight;
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
				m_ghosts[i][0] + 1, 
				m_ghosts[i][1] + 1);
			
			// Number of interfaces (including air before)
			GLint elementLength = (GLint) elementCount;
			
			// Ray grid dimensions
			GLint rayCount = ghostAttribs[i].getMinimumRays();
			
			// Direction of the ray
			glm::vec3 rayDir = glm::vec3(rotMat * glm::vec4(baseDir, 1.0f));
			
			// Center of the ray grid
			glm::vec2 gridCenter = glm::mat2(rotMat) * 
				(ghostAttribs[i].getPupilBounds()[0] + 
				ghostAttribs[i].getPupilBounds()[1] / 2.0f);
			
			// Size of the ray grid
			glm::vec2 gridSize = ghostAttribs[i].getPupilBounds()[1] / 2.0f;
			
			// Center of the ghost image
			glm::vec2 imageCenter = glm::mat2(rotMat) * 
				(ghostAttribs[i].getSensorBounds()[0] + 
				ghostAttribs[i].getSensorBounds()[1] / 2.0f);
			
			// Size of the ghost image
			glm::vec2 imageSize = ghostAttribs[i].getSensorBounds()[1] / 2.0f;
			
			// Distance of the ray from the sensor, along the optical axis
			GLfloat rayDist = sensorDistance + 0.1f;
			
			// Size of the film
			glm::vec2 filmSize = m_opticalSystem->getFilmSize();

			// Wavelength of the ray we are tracing
			GLfloat lambda = m_lambdas[ch];

			// Lambertian coefficient
			GLfloat intensity = lambert * m_intensityScale;

			// Light color
			glm::vec4 color = glm::vec4(light.getDiffuseColor() * 
				light.getDiffuseIntensity(), 1.0f);

			// Ghost rendering mode (ghost, ray grid, etc.)
			GLint renderMode = m_renderMode;

			// Ghost shading mode (shaded, unlit, UV, etc.)
			GLint shadingMode = m_shadeMode;

			// Radius clipping
			GLfloat radiusClip = m_radiusClip;

			// Iris clipping.
			GLfloat irisClip = m_distanceClip;

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
	}
    glBindVertexArray(0);
}

}