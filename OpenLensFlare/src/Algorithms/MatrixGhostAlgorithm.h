#pragma once

#include "../OpticalSystem.h"
#include "../Ghost.h"
#include "../LightSource.h"
#include "../GhostAlgorithm.h"

namespace OLEF
{

/// Implements the matrix optics based ghost rendering algorithm, as described 
/// in the 2013 paper.
class MatrixGhostAlgorithm: public GhostAlgorithm
{
public:
    /// Construct a matrix optics based flare rendering object that can render
    /// the ghosts provided in the parameter ghost list.
    MatrixGhostAlgorithm(OpticalSystem* system, const GhostList& ghosts);
    
    /// Releases all the allocated GL objects.
    ~MatrixGhostAlgorithm();

    /// These objects are not copyable.
    MatrixGhostAlgorithm(const MatrixGhostAlgorithm& other) = delete;

    /// These objects are not copyable.
    MatrixGhostAlgorithm& operator=(const MatrixGhostAlgorithm& other) = delete;
    
    /// Renders the ghosts corresponding to the parameter light source.
    void renderGhosts(const LightSource& light, int first, int last);

    /// Returns the optical system that generates the ghosts.
    OpticalSystem* getOpticalSystem() const { return m_opticalSystem; }

    /// Returns the list of ghosts this object can render.
    const GhostList& getGhostList() const { return m_ghosts; }

private:
    /// Instances of this class describe a particular ghost channel is rendered.
    struct GhostAttributesChannel
    {
        /// Transferred intensity.
        float m_intensity;
        
        /// Transfer matrices that describe how a ray is transferred to the next
        /// relevant surface (aperture or sensor).
        std::vector<glm::mat2> m_matrices;
    };

    /// An object holding all the channel data for a partical ghost.
    using GhostAttributes = std::vector<GhostAttributesChannel>;

    /// Computes the attributes for the parameter ghost.
    GhostAttributes computeGhostAttributes(const Ghost& ghost);

    /// The optical system that generates the ghosts.
    OpticalSystem* m_opticalSystem;

    // The full list of all the ghosts this object should render.
    GhostList m_ghosts;

    /// Wavelengths at which to render the ghosts.
    std::vector<float> m_lambdas;

    /// A dummy vao to use.
    GLuint m_vao;

    /// Shader used for rendering.
    GLuint m_renderShader;
};

}