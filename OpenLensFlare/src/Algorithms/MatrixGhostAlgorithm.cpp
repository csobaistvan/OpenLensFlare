#include "MatrixGhostAlgorithm.h"
#include "GLHelpers.h"

#include "Common_Functions.glsl.h"
#include "Common_ColorSpace.glsl.h"
#include "MatrixGhostAlgorithm_RenderGhost_VertexShader.glsl.h"
#include "MatrixGhostAlgorithm_RenderGhost_FragmentShader.glsl.h"

namespace OLEF
{

////////////////////////////////////////////////////////////////////////////////
MatrixGhostAlgorithm::MatrixGhostAlgorithm(OpticalSystem* system, const GhostList& ghosts):
    m_opticalSystem(system),
    m_ghosts(ghosts),
    m_vao(0)
{
    // Create the render shader
    GLHelpers::ShaderSource renderSource;
    
    renderSource.m_source =
    {
        {
            GL_VERTEX_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
                Shaders::MatrixGhostAlgorithm_RenderGhost_VertexShader,
            }
        },
        {
            GL_FRAGMENT_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
                Shaders::MatrixGhostAlgorithm_RenderGhost_FragmentShader,
            }
        },
    };
    m_renderShader = GLHelpers::createShader(renderSource);

    // Generate a dummy vertex array.
    glGenVertexArrays(1, &m_vao);
}

MatrixGhostAlgorithm::~MatrixGhostAlgorithm()
{
    // Generate a dummy vertex array.
    glDeleteVertexArrays(1, &m_vao);
    
    // Release the shader
    glDeleteProgram(m_renderShader);
}

////////////////////////////////////////////////////////////////////////////////
/*
glm::mat2 translationMatrix(float dist)
{
	glm::mat2 result;

	result[1][0] = dist;

	return result;
}

glm::mat2 reflectionMatrix(float radius)
{
	glm::mat2 result;

	if (radius != 0.0f)
	{
		result[0][1] = 2.0f / radius;
	}

	return result;
}

glm::mat2 refractionMatrix(float n1, float n2, float radius)
{
	glm::mat2 result;

	result[1][1] = n1 / n2;
	if (radius != 0.0f)
	{
		result[0][1] = (n1 - n2) / (n2 * radius);
	}

	return result;
}

float coatingFresnel(float lambda, float coatingLambda, float theta0, float n0, float n1, float n2)
{
	// Compute the optimial coating refractive index
	n1 = glm::max(glm::sqrt(n0 * n2), n1);

	// Compute the optimal coating thickness
	float d = coatingLambda / 4.0f / n1;

	// Apply Snell's law to get the other angles
	float theta1 = glm::asin(glm::sin(theta0) * n0 / n1);
	float theta2 = glm::asin(glm::sin(theta0) * n0 / n2);

	float cos0 = glm::cos(theta0);
	float cos1 = glm::cos(theta1);
	float cos2 = glm::cos(theta2);

	float beta = (glm::two_pi<float>()) / lambda * n1 * d * cos1;

	// Compute the Fresnel terms for the first and second interfaces for both
	// s and p polarized light
	float r12p = (n1 * cos0 - n0 * cos1) / (n1 * cos0 + n0 * cos1);
	float r12p2 = r12p * r12p;

	float r23p = (n2 * cos1 - n1 * cos2) / (n2 * cos1 + n1 * cos2);
	float r23p2 = r23p * r23p;

	float rp = (r12p2 + r23p2 + 2.0f * r12p * r23p * glm::cos(2.0f * beta)) /
		(1.0f + r12p2 * r23p2 + 2.0f * r12p * r23p * glm::cos(2.0f * beta));

	float r12s = (n0 * cos0 - n1 * cos1) / (n0 * cos0 + n1 * cos1);
	float r12s2 = r12s * r12s;

	float r23s = (n1 * cos1 - n2 * cos2) / (n1 * cos1 + n2 * cos2);
	float r23s2 = r23s * r23s;

	float rs = (r12s2 + r23s2 + 2.0f * r12s * r23s * glm::cos(2.0f * beta)) /
		(1.0f + r12s2 * r23s2 + 2.0f * r12s * r23s * glm::cos(2.0f * beta));

	return (rs + rp) * 0.5f;
}

GhostAttributes MatrixGhostAlgorithm::computeGhostAttributes(const GhostList::Ghost& ghost)
{
	// Resulting matrices
	GhostAttributes result;

    result.m_intensity = 1.0f;
    result.m_matrices.resize(m_opticalSystem->getApertureCount())

    // Extract the optical system elements
    const auto& elements = m_opticalSystem->getElements();

	// Do the computations for each channel
	for (int ch = 0; ch < m_lambda.size(); ++ch)
	{
        // Index of the matrix we are appending to currently.
        int matrixId = 0;

        // The direction in which the ray is travelling.
        int direction = 1;

        // Process each interface
        for (int id = 0; i < ghost.size(); ++id)
        {
            // Keep on transferring until we reach the interface
            for (int i = 0; i < ghost[i]; ++i)
            {
                glm::mat2 Ri = refractionMatrix(
                    elements[i - direction].computeIndexOfRefraction(m_lambdas[ch]), 
                    elements[i].computeIndexOfRefraction(m_lambdasch]), 
                    elements[i].getRadiusOfCurvature());
                glm::mat2 Ti = translationMatrix(elements[i].getThickness());
                result[ch].m_matrices[id] = Ti * Ri * result[ch].m_matrices[id];

                // Increment the matrix id upon reaching an aperture.
                if (elements[i].getType() == OpticalSystemElement::ElementType::APERTURE_STOP)
                {
                    matrixId += direction;
                }
            }

            // Reflect off of the component and transfer back
            glm::mat2 L = reflectionMatrix(elements[ghost[i]].getRadiusOfCurvature());
            glm::mat2 T = translationMatrix(elements[ghost[i] - 1].getThickness());
            result[ch].m_matrices[id] = T * L * result[ch].m_matrices[id];

            float I = coatingFresnel(
                m_lambdas[ch], 
                elements[ghost[i]].getCoatingLambda(), 
                0.0f, 
                elements[ghost[i] - 1].computeIndexOfRefraction(m_lambdas[ch]),
                elements[ghost[i]].getCoatingIor(), 
                elements[ghost[i]].computeIndexOfRefraction(m_lambdas[ch]);
            result[ch].m_intensity *= I;
        }
	}

	// Return the result
	return result;
}
*/
void MatrixGhostAlgorithm::renderGhosts(const LightSource& light, int first, int last)
{
    /*
    // Angle of light that we are tracing
    float theta = glm::acos(glm::dot(glm::vec3(0, 0, -1), light.getIncidenceDirection()));
    
    // Compute the system matrices
    auto systemMatrices = getMatrixLensFlareSystemMatrices(object);

    // Effective aperture height
    float H_ea = (m_opticalSystem->getEffectiveFocalLength() / 
        m_opticalSystem->getFnumber()) / 2.0f;

    // Compute the physical aperture height
    float H_ep = H_ea / systemMatrices.first[0][0];

    // Film size
    float H_f = m_opticalSystem->getFilmSize().y * 0.5f;

    // Aspect correction vector
    glm::vec2 aspectCorrection = glm::vec2(1.0f, 1.0f);

    // Bind the render shader
    glUseProgram(m_renderShader);

    // Bind the aperture texture
    glActiveTexture(GL_TEXTURE0 + TEXTURE_POST_PROCESS_1);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLuint textureLoc = glGetUniformLocation(m_renderShader, "sAperture");
    glUniform1i(textureLoc, 0);

    // Go through the ghost list and render them
    for (int i = first; i <= last; ++i)
    {
        // Compute the ghost matrices
        auto ghostData = getMatrixLensFlareGhostData(object, ghostIndices[i][0], ghostIndices[i][1], theta);

        // Compute the ghost projections per-channel
        std::vectors<float> centers(m_lambdas.size());
        std::vector<float> radi(m_lambdas.size());
        std::vector<float> intensities(m_lambdas.size());
        glm::vec2 loosePupilsizes, looseProjections;

        for (size_t ch = 0; ch < m_lambdas.size(); ++ch)
        {
            // Compute the height of the ghost's effective pupil
            glm::vec2 pupilSizes =
            {
                ( H_ea - ghostData[ch].m_matrices[0][1][0] * theta) / ghostData[ch].m_matrices[0][0][0],
                (-H_ea - ghostData[ch].m_matrices[0][1][0] * theta) / ghostData[ch].m_matrices[0][0][0], 
            };
            if (pupilSizes[1] < pupilSizes[0])
                std::swap(pupilSizes[0], pupilSizes[1]);

            // Project it onto the sensor plane
            glm::vec2 projections =
            {
                ((ghostData[ch].m_matrices[1] * ghostData[ch].m_matrices[0] * glm::vec2(pupilSizes[0], theta)).x) / H_f,
                ((ghostData[ch].m_matrices[1] * ghostData[ch].m_matrices[0] * glm::vec2(pupilSizes[1], theta)).x) / H_f,
            };
            if (projections[1] < projections[0])
                std::swap(projections[0], projections[1]);

            // Compute the center and radius
            centers[ch] = (projections[0] + projections[1]) / 2.0f;
            radi[ch] = (projections[1] - projections[0]) / 2.0f;
            intensities[ch] = ghostData.m_channels[ch].m_intensity * 
                ((glm::pow(pupilSizes[0] - pupilSizes[1], 2.0f) / glm::pow(H_ep * 2.0f, 2.0f)) / 
                glm::pow(projections[0] - projections[1], 2.0f));

            // Initialize the loose coordinates with these if this is the first channel
            if (ch == 0)
            {
                loosePupilsizes[0] = pupilSizes[0];
                loosePupilsizes[1] = pupilSizes[1];
                looseProjections[0] = projections[0];
                looseProjections[1] = projections[1];
            }
            
            // Compute the loose projections
            loosePupilsizes[0] = glm::min(pupilSizes[0], loosePupilsizes[0]);
            loosePupilsizes[1] = glm::max(pupilSizes[1], loosePupilsizes[1]);
            looseProjections[0] = glm::min(projections[0], looseProjections[0]);
            looseProjections[1] = glm::max(projections[1], looseProjections[1]);
        }

        // Compute the loose flare quad
        float center = (looseProjections[0] + looseProjections[1]) / 2.0f;
        float radius = (looseProjections[1] - looseProjections[0]) / 2.0f;

        // Upload the loose parameters
        glm::vec2 position = center * toLightScreenSpace;
        glm::vec2 scale = radius * aspectCorrection;
        glm::vec3 color = glm::vec3(intensities[0], intensities[1], intensities[2]) * 
            light.getDiffuseColor() * light.getDiffuseIntensity();

        // Upload the channel parameters
        std::vector<glm::vec2> channelPositions(m_lambdas.size());
        std::vector<glm::vec2> channelScales(m_lambdas.size());
        std::vector<glm::vec3> channelColors(m_lambdas.size());
        for (size_t i = 0; i < m_lambdas; ++i)
        {
            channelPositions = (((center * toLightScreenSpace) - glm::vec2(radius)) -
                ((centers[i] * toLightScreenSpace) - glm::vec2(radi[i]))) / (radius * 2.0f);
            channelScales[i] = glm::vec2(radius / radi[i]);
            channelColors[i] = glm::vec3(1.0f);
        }

        // Render the plane mesh
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    */
}

}