#include "RayTraceGhostAlgorithm.h"
#include "GLHelpers.h"

#include "Common_Functions.glsl.h"
#include "Common_ColorSpace.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_Uniforms.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_VertexShader.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_GeometryShader.glsl.h"
#include "RayTraceGhostAlgorithm_RenderGhost_FragmentShader.glsl.h"

//TODO: implement two precomputation methods: transform feedback and compute
//      shader versions, and expose a switch or something to allow the user
//      choose from the desired version
//TODO: extract the ray tracing code into a separate file and assemble the 
//      required shaders (rendering, xfb precomputation, compute shader
//      precomputation) using the multiple source file feature of the shader 
//      compiler function

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
	parameters.m_shader = m_precomputeShader;
	parameters.m_intensityScale = 1.0f;
	parameters.m_renderMode = RenderMode::PROJECTED_GHOST;
	parameters.m_shadingMode = ShadingMode::SHADED;
	parameters.m_radiusClip = 100000.0f;
	parameters.m_distanceClip = 100000.0f;
	
	// Highest ray grid size that is used while bounding
	int maxRaysBounds = *std::max_element(
		computeParams.m_boundingRays.begin(), computeParams.m_boundingRays.end());
	int maxRaysPresets = *std::max_element(
		computeParams.m_rayPresets.begin(), computeParams.m_rayPresets.end());
	int maxRays = std::max(maxRaysBounds, maxRaysPresets);

	// Compute the maximum buffer size needed, and per-ghost byte offsets into
	// the buffer
	std::vector<std::array<int, 2>> byteOffsets(ghosts.size());
	std::vector<std::array<int, 2>> vertexOffsets(ghosts.size());
	GLint vertexSize = 0;
	GLint bufferSize = 0;
	
	for (size_t ghostId = 0; ghostId < ghosts.size(); ++ghostId)
	{
		// Skip invalid ghosts
		if (!m_opticalSystem->isValidGhost(ghosts[ghostId]))
		{
			continue;
		}

		// Per-channel vertices and bytes needed
		int vertices = (maxRays - 1) * (maxRays - 1) * 6;
		int bytes = vertices * sizeof(PerVertexData);

		// Total vertices and bytes per ghost
		int totalVerts = vertices * (int) computeParams.m_lambdas.size();
		int totalBytes = bytes * (int) computeParams.m_lambdas.size();

		// Per-channel offsets and sizes
		vertexOffsets[ghostId] = { vertexSize, vertices };
		byteOffsets[ghostId] = { bufferSize, bytes };

		// Total number of vertices and bytes needed
		vertexSize += totalVerts;
		bufferSize += totalBytes;
	}

	// Create and initialize the read-back buffer
	GLuint readBackBuffer;
	glGenBuffers(1, &readBackBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, readBackBuffer);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_READ);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Bind the precomputation shader
	glUseProgram(m_precomputeShader);
    glBindVertexArray(m_vao);

	// Disable rasterization
	glEnable(GL_RASTERIZER_DISCARD);

	// Compute ghost bounding information
	for (int passId = 0; passId < computeParams.m_boundingRays.size(); ++passId)
	{
		// Extract the current grid size
		int numRays = computeParams.m_boundingRays[passId];
		int numVertices = (numRays - 1) * (numRays - 1) * 6;

		// Set it as the fixed ray grid size
		parameters.m_fixedRayCount = numRays;

		// Process each ghost
		for (int ghostId = 0; ghostId < ghosts.size(); ++ghostId)
		{
			// Skip invalid or previously detected invisible ghosts
			if (!m_opticalSystem->isValidGhost(result[ghostId]) || 
				result[ghostId].getPupilBounds()[1][0] < 0.0f)
			{
				continue;
			}

			// Set the ghost we are rendering
			parameters.m_ghost = result[ghostId];

			// Process each channel
			for (int chId = 0; chId < computeParams.m_lambdas.size(); ++chId)
			{
				// Set the current wavelength
				parameters.m_lambda = computeParams.m_lambdas[chId];

				// Extract the corresponding byte offset and byte size
				auto byteOffset = byteOffsets[ghostId][0];
				auto byteSize = byteOffsets[ghostId][1];

				// Bind the transform feedback buffer
				glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 
					readBackBuffer, byteOffset + chId * byteSize, byteSize);

				// Start the transform feedback
				glBeginTransformFeedback(GL_TRIANGLES);

				// Render the ghost
				renderGhostChannel(parameters);

				// End the transform feedback
				glEndTransformFeedback();
			}
		}
		
		// Read back the values
		glMemoryBarrier(GL_TRANSFORM_FEEDBACK_BARRIER_BIT);
		glBindBuffer(GL_ARRAY_BUFFER, readBackBuffer);
		PerVertexData* vertices = 
			(PerVertexData*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

		// Process the generated ray data to find the bounds
		for (int ghostId = 0; ghostId < ghosts.size(); ++ghostId)
		{
			// Output bounding information - note that this is temporarily stored
			// in a min-max corner format, instead of corner-size, to help
			// with the computations
			Ghost::BoundingRect pupilBounds = { glm::vec2(1.0f), glm::vec2(-1.0f) };
			Ghost::BoundingRect sensorBounds = { glm::vec2(1.0f), glm::vec2(-1.0f) };

			// Go through each channel
			for (int channelId = 0; channelId < computeParams.m_lambdas.size(); ++channelId)
			{
				// Process each triangle
				for (int triangleId = 0; triangleId < numVertices / 3; ++triangleId)
				{
					// Index of the first vertex
					int baseVertexId = vertexOffsets[ghostId][0] + 
						channelId * vertexOffsets[ghostId][1] + triangleId * 3;

					// Keep the full triangle if any of its vertices are 'valid'
					bool keep = false;
					for (int vertexId = 0; vertexId < 3; ++vertexId)
					{
						// Extract the current vertex
						int actualVertexId = baseVertexId + vertexId;
						const auto& vertex = vertices[actualVertexId];

						keep = keep || (
							vertex.m_radius <= computeParams.m_radiusClip && 
							vertex.m_intensity >= computeParams.m_intensityClip && 
							vertex.m_irisDistance <= computeParams.m_distanceClip);
					}

					// Skip the full triangle if all of its vertices are invalid
					if (!keep)
						continue;

					// Update the bounds using all 3 vertices
					for (int vertexId = 0; vertexId < 3; ++vertexId)
					{
						// Extract the current vertex
						int actualVertexId = baseVertexId + vertexId;
						const auto& vertex = vertices[actualVertexId];

						pupilBounds[0] = glm::min(pupilBounds[0], vertex.m_parameter);
						pupilBounds[1] = glm::max(pupilBounds[1], vertex.m_parameter);

						sensorBounds[0] = glm::min(sensorBounds[0], vertex.m_position);
						sensorBounds[1] = glm::max(sensorBounds[1], vertex.m_position);
					}
				}
			}

			// Make sure the ghost is visible
			if (pupilBounds[1][0] < pupilBounds[0][0] ||
				(pupilBounds[0][0] > 1.0f && pupilBounds[0][1] > 1.0f) || 
				(pupilBounds[1][0] < -1.0f && pupilBounds[1][1] < -1.0f))
			{
				pupilBounds[0] = pupilBounds[1] = glm::vec2(-1.0f);
				sensorBounds[0] = sensorBounds[1] = glm::vec2(-1.0f);

				result[ghostId].setPupilBounds(pupilBounds);
				result[ghostId].setSensorBounds(sensorBounds);
			}

			// Store the computed bounds
			else
			{
				pupilBounds[1] = pupilBounds[1] - pupilBounds[0];
				sensorBounds[1] = sensorBounds[1] - sensorBounds[0];

				result[ghostId].setPupilBounds(pupilBounds);
				result[ghostId].setSensorBounds(sensorBounds);
			}
		}

		// Unmap the buffer
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Create the vectors that we will be using during the bounding process
	std::vector<float> triangleAreas;   // Holds the area of each valid triangle
	std::vector<float> areaDifferences; // Holds the difference between each
	                                    // triangle area and the average area
	std::vector<float> totalAreas;      // Holds the total areas for each channel
	std::vector<float> avgAreas;        // Holds the average areas for each channel
	std::vector<float> variances;       // Holds the per-channel variances

	triangleAreas.reserve((maxRaysPresets - 1) * (maxRaysPresets - 1) * 2);
	areaDifferences.reserve((maxRaysPresets - 1) * (maxRaysPresets - 1) * 2);
	totalAreas.reserve(computeParams.m_lambdas.size());
	avgAreas.reserve(computeParams.m_lambdas.size());
	variances.reserve(computeParams.m_lambdas.size());

	// Set the ray grid sizes to 0 for each ghost, to indicate that it needs
	// to be processed
	for (int ghostId = 0; ghostId < ghosts.size(); ++ghostId)
	{
		result[ghostId].setMinimumRays(0);
		result[ghostId].setOptimalRays(0);
	}

	// Compute bounding geometry
	for (int passId = 0; passId < computeParams.m_rayPresets.size(); ++passId)
	{
		// Extract the current grid size
		int numRays = computeParams.m_rayPresets[passId];
		int numVertices = (numRays - 1) * (numRays - 1) * 6;

		// Set it as the fixed ray grid size
		parameters.m_fixedRayCount = numRays;

		// Process each ghost
		for (int ghostId = 0; ghostId < ghosts.size(); ++ghostId)
		{
			// Skip invalid, invisible, and already finished ghosts
			if (!m_opticalSystem->isValidGhost(result[ghostId]) || 
				result[ghostId].getPupilBounds()[1][0] < 0.0f ||
				result[ghostId].getMinimumRays() != 0)
			{
				continue;
			}

			// Set the ghost we are rendering
			parameters.m_ghost = result[ghostId];

			// Process each channel
			for (int chId = 0; chId < computeParams.m_lambdas.size(); ++chId)
			{
				// Set the current wavelength
				parameters.m_lambda = computeParams.m_lambdas[chId];

				// Extract the corresponding byte offset and byte size
				auto byteOffset = byteOffsets[ghostId][0];
				auto byteSize = byteOffsets[ghostId][1];

				// Bind the transform feedback buffer
				glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 
					readBackBuffer, byteOffset + chId * byteSize, byteSize);

				// Start the transform feedback
				glBeginTransformFeedback(GL_TRIANGLES);

				// Render the ghost
				renderGhostChannel(parameters);

				// End the transform feedback
				glEndTransformFeedback();
			}
		}
		
		// Read back the values
		glMemoryBarrier(GL_TRANSFORM_FEEDBACK_BARRIER_BIT);
		glBindBuffer(GL_ARRAY_BUFFER, readBackBuffer);
		PerVertexData* vertices = 
			(PerVertexData*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

		// Process the generated ray data to find the rest of the attributes
		for (int ghostId = 0; ghostId < ghosts.size(); ++ghostId)
		{
			// Skip invalid, invisible, and already finished ghosts
			if (!m_opticalSystem->isValidGhost(result[ghostId]) || 
				result[ghostId].getPupilBounds()[1][0] < 0.0f ||
				result[ghostId].getMinimumRays() != 0)
			{
				continue;
			}

            // Clear the variance vector
            totalAreas.clear();
            avgAreas.clear();
            variances.clear();
			float totalIntensity = 0.0f;
			int validVertices = 0;

			// Go through each channel
			for (int channelId = 0; channelId < computeParams.m_lambdas.size(); ++channelId)
			{
				// Clear the temporary buffer holding triangle information
				triangleAreas.clear();

				// Process each vertex
				for (int vertexId = 0; vertexId < numVertices; ++vertexId)
				{
					int actualVertexId = vertexOffsets[ghostId][0] + 
						channelId * vertexOffsets[ghostId][1] + vertexId;
					const auto& vertex = vertices[actualVertexId];

					if (vertex.m_radius <= computeParams.m_radiusClip && 
						vertex.m_intensity >= computeParams.m_intensityClip && 
						vertex.m_irisDistance <= computeParams.m_distanceClip)
					{
						totalIntensity += vertex.m_intensity;
						++validVertices;
					}
				}

				// Process each triangle
				for (int triangleId = 0; triangleId < numVertices / 3; ++triangleId)
				{
					// Index of the first vertex
					int baseVertexId = vertexOffsets[ghostId][0] + 
						channelId * vertexOffsets[ghostId][1] + triangleId * 3;

					// Only keep those triangles that are fully valid (a.k.a.
					// all of its vertices are valid) - this should minimize
					// the effect of degenerate values on the output
					bool keep = true;
					for (int vertexId = 0; vertexId < 3; ++vertexId)
					{
						// Extract the current vertex
						int actualVertexId = baseVertexId + vertexId;
						const auto& vertex = vertices[actualVertexId];

						keep = keep && (
							vertex.m_radius <= computeParams.m_radiusClip && 
							vertex.m_intensity >= computeParams.m_intensityClip && 
							vertex.m_irisDistance <= computeParams.m_distanceClip);
					}

					// Skip a degenerate triangle
					if (!keep)
					{
						continue;
					}

					// Extract the triangle vertices
					glm::vec2 triVertices[] =
					{
						vertices[baseVertexId + 0].m_position,
						vertices[baseVertexId + 1].m_position,
						vertices[baseVertexId + 2].m_position,
					};
					
					// Compute the area of the projected triangle
					float triangleArea = 0.5f * glm::abs(
						triVertices[0].x * (triVertices[1].y - triVertices[2].y) +
						triVertices[1].x * (triVertices[2].y - triVertices[0].y) +
						triVertices[2].x * (triVertices[0].y - triVertices[1].y));

					// Store it
					triangleAreas.push_back(triangleArea);
				}
				
				// Skip the remaining computations if the channel is fully invisible
				if (triangleAreas.empty())
					continue;

				// Compute the total area
				float totalArea = std::accumulate(
					triangleAreas.begin(), triangleAreas.end(), 0.0f);
				totalAreas.push_back(totalArea);

				// Compute the avg area
				float avgArea = totalArea / triangleAreas.size();
				avgAreas.push_back(avgArea);

				// Compute the variance
				areaDifferences.resize(triangleAreas.size());
				std::transform(
					triangleAreas.begin(), triangleAreas.end(), areaDifferences.begin(), 
					[&] (float area) { return glm::pow(area - avgArea, 2.0f); });

				float totalVariance = std::accumulate(
					areaDifferences.begin(), areaDifferences.end(), 0.0f);
				float variance = glm::sqrt(totalVariance / areaDifferences.size());
				variances.push_back(variance);
			}

			// Compute the average variance
			float avgVariance = std::accumulate(
				variances.begin(), variances.end(), 0.0f) / variances.size();

			// Area of an 'ideal' triangle - that is, if the ghost projection
			// was equally distributed over the sensor bounds, then this would
			// be the area of a single triangle cell
			float cellArea = 0.5f * 
				(result[ghostId].getSensorBounds()[1].x / (numRays - 1)) *
				(result[ghostId].getSensorBounds()[1].y / (numRays - 1));

			// Compute the average variance's ratio to the ideal cell area
			float varianceToCellArea = avgVariance / cellArea;

			// The current variance value to use for comparison.
            //float currentVariance = varianceToCellArea;
			float currentVariance = avgVariance;

			// Store the grid size as the result if the variance is small enough
			if (currentVariance <= computeParams.m_targetVariance ||
				numRays == computeParams.m_rayPresets.back())
			{
				result[ghostId].setMinimumRays(numRays);
				result[ghostId].setOptimalRays(numRays);
				result[ghostId].setAverageIntensity(totalIntensity / validVertices);
			}
		}

		// Unmap the buffer
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Re-enable rasterization
	glDisable(GL_RASTERIZER_DISCARD);
    glBindVertexArray(0);

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
	GLHelpers::uploadUniform(parameters.m_shader, "sAperture", 0);

	// Temporary vectors for the lens parameters
	auto elementCount = m_opticalSystem->getElementCount() + 1;

	static const int MAX_ELEMENTS = 64;

	float heights[MAX_ELEMENTS] = { 0.0f };
	float curvatures[MAX_ELEMENTS] = { 0.0f };
	float apertures[MAX_ELEMENTS] = { 0.0f };
	float thicknesses[MAX_ELEMENTS] = { 0.0f };
	glm::vec3 centers[MAX_ELEMENTS] = { glm::vec3(0.0f) };
	glm::vec3 refractions[MAX_ELEMENTS] = { glm::vec3(1.0f) };

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
	GLint ghostIndices[16];
	for (int i = 0; i < parameters.m_ghost.getLength(); ++i)
	{
		ghostIndices[i] = parameters.m_ghost[i] + 1;
	}

	// Number of interfaces (including air before)
	GLint numIndices = (GLint) parameters.m_ghost.getLength();
	
	// Number of interfaces (including air before)
	GLint elementLength = (GLint) elementCount;
	
	// Ray grid dimensions
	GLint rayCount = parameters.m_fixedRayCount != 0 ? 
		parameters.m_fixedRayCount :
		parameters.m_ghost.getMinimumRays();
	
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
	GLHelpers::uploadUniform(parameters.m_shader, "vLensCenter", centers);
	GLHelpers::uploadUniform(parameters.m_shader, "vLensIor", refractions);
	GLHelpers::uploadUniform(parameters.m_shader, "fLensRadius", curvatures);
	GLHelpers::uploadUniform(parameters.m_shader, "fLensHeight", heights);
	GLHelpers::uploadUniform(parameters.m_shader, "fLensAperture", apertures);
	GLHelpers::uploadUniform(parameters.m_shader, "fLensCoating", thicknesses);

	GLHelpers::uploadUniform(parameters.m_shader, "iGhostIndices", ghostIndices);
	GLHelpers::uploadUniform(parameters.m_shader, "iNumIndices", numIndices);
	GLHelpers::uploadUniform(parameters.m_shader, "iLength", elementLength);
	GLHelpers::uploadUniform(parameters.m_shader, "iRayCount", rayCount);
	GLHelpers::uploadUniform(parameters.m_shader, "vRayDir", rayDir);
	GLHelpers::uploadUniform(parameters.m_shader, "vGridCenter", gridCenter);
	GLHelpers::uploadUniform(parameters.m_shader, "vGridSize", gridSize);
	GLHelpers::uploadUniform(parameters.m_shader, "vImageCenter", imageCenter);
	GLHelpers::uploadUniform(parameters.m_shader, "vImageSize", imageSize);
	GLHelpers::uploadUniform(parameters.m_shader, "fRayDistance", rayDist);
	GLHelpers::uploadUniform(parameters.m_shader, "vFilmSize", filmSize);
	GLHelpers::uploadUniform(parameters.m_shader, "fLambda", lambda);
	GLHelpers::uploadUniform(parameters.m_shader, "fIntensityScale", intensity);
	GLHelpers::uploadUniform(parameters.m_shader, "vColor", color);
	GLHelpers::uploadUniform(parameters.m_shader, "iRenderMode", renderMode);
	GLHelpers::uploadUniform(parameters.m_shader, "iShadingMode", shadingMode);
	GLHelpers::uploadUniform(parameters.m_shader, "fRadiusClip", radiusClip);
	GLHelpers::uploadUniform(parameters.m_shader, "fIrisClip", irisClip);

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
	parameters.m_shader = m_renderShader;
	parameters.m_fixedRayCount = 0;
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