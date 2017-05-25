#pragma once

#include "../OpticalSystem.h"
#include "../GhostList.h"
#include "../GhostAttributes.h"
#include "../LightSource.h"
#include "../GhostAlgorithm.h"

namespace OLEF
{

/// Implements ray traced ghost rendering, as described in the 2011 paper.
class RayTraceGhostAlgorithm: public GhostAlgorithm
{
public:
    /// Construct an empty instance. Use deserialize() to populate to the object
    /// with all the data needed to be functional.
    RayTraceGhostAlgorithm();

    /// Construct a ray traced flare rendering object that can render the
    /// ghosts provided in the parameter object.
    RayTraceGhostAlgorithm(OpticalSystem* system, const GhostList& ghosts);

    /// Construct a ray traced flare rendering object that can render the
    /// ghosts provided in the parameter object.
    RayTraceGhostAlgorithm(OpticalSystem* system, const GhostList& ghosts,
        const std::vector<float>& lambdas, float intensityScaling);
    
    /// Releases all the allocated GL objects.
    ~RayTraceGhostAlgorithm();

    /// These objects are not copyable.
    RayTraceGhostAlgorithm(const RayTraceGhostAlgorithm& other) = delete;

    /// These objects are not copyable.
    RayTraceGhostAlgorithm& operator=(const RayTraceGhostAlgorithm& other) = delete;

    /// Helper structure holding all the parameters needed for computing
    /// the rendering attributes of a ghost.
    struct ComputeParams
    {
        /// The first angle to compute data for. 
        float m_firstAngle = 0.0f;

        /// The angle difference between two consecutive computation. 
        float m_angleStep = 0.5f;

        /// The total number of angles.
        int m_numAngles = 181;

        /// The total number of bounding passes to take per ghost.
        int m_boundingPasses = 2;

        /// The number of rays to use in each bounding step. The last element
        /// is used if the number of passes excreeds the vector size.
        std::vector<int> m_boundingRays = { 33, 33 };

        /// The list of ray grid sizes to choose from.
        std::vector<int> m_rayPresets = { 5, 17, 33, 65, 127 };
    };

    /// Computes the ghost rendering attributes corresponding to the provided
    /// parameters, for all the ghosts that the object can render.
    void computeGhostAttributes(const ComputeParams& params = {});

    /// Serializes all the contained data (optical system, list of ghosts and 
    /// their computed attributes) into the target stream. Note that the stream 
    /// is expected to be binary.
    bool serialize(const std::ostream& stream) const;

    /// Deserializes all the contained data from the parameter stream, 
    /// overwriting all the existing data currently in the object. Note that the
    /// stream is expected to be binary.
    bool deserialize(const std::istream& stream);

    /// Renders the ghosts corresponding to the parameter light source.
    void renderGhosts(const LightSource& light, int first, int count);

    /// Returns the optical system that generates the ghosts.
    OpticalSystem* getOpticalSystem() const { return m_opticalSystem; }

    /// Returns the list of ghosts this object can render.
    const GhostList& getGhostList() const { return m_ghosts; }

    /// Returns the intensity scaling factor.
    float getIntensityScale() const { return m_intensityScale; }

    /// Returns the render mode.
    int getRenderMode() const { return m_renderMode; }

    /// Returns the shading mode.
    int getShadeMode() const { return m_shadeMode; }

    /// Returns the radius clipping value.
    float getRadiusClip() const { return m_radiusClip; }

    /// Returns the distance clipping value.
    float getDistanceClip() const { return m_distanceClip; }

    /// Returns the wavelengths at which to render the ghosts.
    const std::vector<float>& getLambdas() const { return m_lambdas; }

    /// Refreshes the ghost list.
    void setGhostList(const GhostList& value) { m_ghosts = value; }

    /// Sets the intensity scaling factor.
    void setIntensityScale(float value) { m_intensityScale = value; }

    /// Sets the render mode.
    void setRenderMode(int value) { m_renderMode = value; }

    /// Sets the shading mode.
    void setShadeMode(int value) { m_shadeMode = value; }

    /// Sets the radius clipping value.
    void setRadiusClip(float value) { m_radiusClip = value; }

    /// Sets the distance clipping value.
    void setDistanceClip(float value) { m_distanceClip = value; }

    /// Sets the wavelengths at which to render the ghosts.
    void setLambdas(const std::vector<float>& value) { m_lambdas = value; }

private:
    void renderGhost();

    /// The optical system that generates the ghosts.
    OpticalSystem* m_opticalSystem;

    // The full list of all the ghosts this object should render.
    GhostList m_ghosts;

    /// A list of ghost attributes, each corresponding to the same angle.
    using GhostAttributesList = std::vector<GhostAttributes>;

    /// List of pre-computed ghost attributes.
    std::map<float, GhostAttributesList> m_attributes;

    /// Intensity scaling.
    float m_intensityScale;

    /// Render mode.
    int m_renderMode;

    /// Shading mode.
    int m_shadeMode;

    /// Radius clipping.
    float m_radiusClip;

    /// Distance clipping.
    float m_distanceClip;

    /// Wavelengths at which to render the ghosts.
    std::vector<float> m_lambdas;

    /// A dummy vao to use.
    GLuint m_vao;
    
    /// Shader used for rendering.
    GLuint m_renderShader;
};

}