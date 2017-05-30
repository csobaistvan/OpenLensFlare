#pragma once

#include "../OpticalSystem.h"
#include "../Ghost.h"
#include "../LightSource.h"
#include "../GhostAlgorithm.h"

namespace OLEF
{

/// Implements ray traced ghost rendering, as described in the 2011 paper.
class RayTraceGhostAlgorithm: public GhostAlgorithm
{
public:
    /// Enumerates the various ways rays can be transformed during rendering to
    /// get their final projected coordinates.
    enum class RenderMode
    {
        /// Use the ray-traced, projected ghost coordinates (normal).
        PROJECTED_GHOST,

        /// Render with the originating ray pupil coordinates.
        PUPIL_GRID,
    };

    /// Enumerates the possible shading mode, which control how the projected
    /// ray geometry is colored.
    enum class ShadingMode
    {
        /// Fully shaded using the target color, intensity scaling and
        /// transmitted energy factor.
        SHADED,

        /// Shaded using the intensity scaling and transmitted energy, but
        /// ignoring the target color.
        UNCOLORED,

        /// Fully white.
        UNSHADED,

        /// Display the pupil coordinates of the correspondng ray.
        RAY_COORDINATES,

        /// Display the UV coordinates of the ray.
        UV_COORDINATES,

        /// Display the corresponding relative radius.
        RELATIVE_RADIUS
    };

    /// Construct a ray traced flare rendering object that can render ghosts
    /// for the parameter optical system.
    RayTraceGhostAlgorithm(OpticalSystem* system);
    
    /// Releases all the allocated GL objects.
    ~RayTraceGhostAlgorithm();

    /// These objects are not copyable.
    RayTraceGhostAlgorithm(const RayTraceGhostAlgorithm& other) = delete;

    /// These objects are not copyable.
    RayTraceGhostAlgorithm& operator=(const RayTraceGhostAlgorithm& other) = delete;

    /// Helper structure holding all the parameters needed for computing
    /// the rendering attributes of a ghost.
    ///
    /// TODO: these should really use an array instead of vectors, to avoid
    ///       dynamic memory allocations (even in such a relatively rarely used
    ///       context).
    struct GhostAttribComputeParams
    {
        /// The incoming angle to compute data for. 
        float m_angle = 0.0f;

        /// Wavelengths used during precomputations
        std::vector<float> m_lambdas = { 650.0f, 510.0f, 475.0f };

        /// The number of rays to use in each bounding step. The size of the
        /// vector also determines the number of bounding steps.
        std::vector<int> m_boundingRays = { 32, 32 };

        /// The list of ray grid sizes to choose from.
        std::vector<int> m_rayPresets = { 5, 16, 32, 64, 128 };

        /// Radius clipping used for the vertices.
        float m_radiusClip = 1.0001f;

        /// Distance clipping of the vertices.
        float m_distanceClip = 0.95001f;

        /// Intensity clipping for the vertices.
        float m_intensityClip = 0.00025f;

        /// The maximum variance across ghost triangles to accept a preset.
        /// Note that the algorithm works by computing the average variance
        /// of the projected ghost images, which is divided by the area of an
        /// ideal cell triangle. The targeted variance is compared against this
        /// value to determine if a preset is suitable.
        float m_targetVariance = 0.01f;
    };

    /// Computes the ghost rendering attributes corresponding to the provided
    /// parameters, and returns a new ghost list with the ghosts containing
    /// the computed attributes.
    GhostList computeGhostAttributes(
        const GhostList& ghosts, const GhostAttribComputeParams& params = {});

    /// Renders the ghosts corresponding to the parameter light source.
    void renderGhosts(const LightSource& light, const GhostList& ghosts);

    /// Returns the optical system that generates the ghosts.
    OpticalSystem* getOpticalSystem() const { return m_opticalSystem; }

    /// Returns the intensity scaling factor.
    float getIntensityScale() const { return m_intensityScale; }

    /// Returns the render mode.
    RenderMode getRenderMode() const { return m_renderMode; }

    /// Returns the shading mode.
    ShadingMode getShadingMode() const { return m_shadingMode; }

    /// Returns the radius clipping value.
    float getRadiusClip() const { return m_radiusClip; }

    /// Returns the distance clipping value.
    float getDistanceClip() const { return m_distanceClip; }

    /// Returns the intensity clipping value.
    float getIntensityClip() const { return m_intensityClip; }

    /// Returns the wavelengths at which to render the ghosts.
    const std::vector<float>& getLambdas() const { return m_lambdas; }

    /// Sets the intensity scaling factor.
    void setIntensityScale(float value) { m_intensityScale = value; }

    /// Sets the render mode.
    void setRenderMode(RenderMode value) { m_renderMode = value; }

    /// Sets the shading mode.
    void setShadingMode(ShadingMode value) { m_shadingMode = value; }

    /// Sets the radius clipping value.
    void setRadiusClip(float value) { m_radiusClip = value; }

    /// Sets the distance clipping value.
    void setDistanceClip(float value) { m_distanceClip = value; }

    /// Sets the intensity clipping value.
    void setIntensityClip(float value) { m_intensityClip = value; }

    /// Sets the wavelengths at which to render the ghosts.
    void setLambdas(const std::vector<float>& value) { m_lambdas = value; }

private:
    /// Parameters used for rendering the ghost.
    struct RenderParameters
    {
        /// The light source to render.
        LightSource m_lightSource;

        /// The ghost to render.
        Ghost m_ghost;

        /// The shader to render with.
        GLuint m_shader;

        /// Mask texture.
        GLuint m_mask;

        /// A fixed ray grid size to use, mainly for precomputation purposes.
        int m_fixedRayCount;

        /// Wavelength to render at.
        float m_lambda;

        /// Intensity scaling.
        float m_intensityScale;

        /// Render mode.
        RenderMode m_renderMode;

        /// Shading mode.
        ShadingMode m_shadingMode;

        /// Radius clipping.
        float m_radiusClip;

        /// Distance clipping.
        float m_distanceClip;
    };

    /// Per-vertex data, read back through transform feedback.
    struct PerVertexData
    {
        /// Position of the ray on the pupil.
        glm::vec2 m_parameter;

        /// Position of the ray's projection on the sensor.
        glm::vec2 m_position;

        /// UV coordinates of the ray's hit on the iris.
        glm::vec2 m_uv;

        /// Distance of the trace hit from the optical axis.
        GLfloat m_radius;

        /// Transmitted light intensity of the ghost.
        GLfloat m_intensity;

        /// Distance of the ray to the center of the iris.
        GLfloat m_irisDistance;
    };

    /// Renders a specific channel of a ghost. It uses a parameter structure
    /// so that it can be reused for both rendering and parameter computation.
    void renderGhostChannel(const RenderParameters& parameters);

    /// The optical system that generates the ghosts.
    OpticalSystem* m_opticalSystem;

    /// Intensity scaling.
    float m_intensityScale;

    /// Render mode.
    RenderMode m_renderMode;

    /// Shading mode.
    ShadingMode m_shadingMode;

    /// Radius clipping.
    float m_radiusClip;

    /// Distance clipping.
    float m_distanceClip;

    /// Intensity clip value, used to reject low intensity ghosts.
    float m_intensityClip;

    /// Wavelengths at which to render the ghosts.
    std::vector<float> m_lambdas;

    /// A dummy vertex array to use, since OpenGL requires a valid object to be
    /// bound, even if we don't actually use any vertex buffers.
    GLuint m_vao;

    /// Shader used for parameter computation.
    GLuint m_precomputeShader;
    
    /// Shader used for rendering.
    GLuint m_renderShader;
};

}