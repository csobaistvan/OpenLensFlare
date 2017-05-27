#pragma once

#include "Dependencies.h"
#include "Ghost.h"

namespace OLEF
{

/// Describes an optical system element. Length attributes are measured in 
/// millimeters, while wavelengths are always measured in nanometers.
class OpticalSystemElement
{
public:
    /// Type of the element.
    enum class ElementType
    {
        /// A normal, spherical lens element.
        LENS_SPHERICAL,
        
        /// An aspherical lens element.
        LENS_ASPHERICAL,
        
        /// The/an aperture stop.
        APERTURE_STOP,
        
        /// The sensor/film.
        SENSOR
    };

    /// Constructs an 'empty' optical element.
    OpticalSystemElement():
        m_type(ElementType::LENS_SPHERICAL),
        m_height(0.0f),
        m_thickness(0.0f),
        m_radiusOfCurvature(0.0f),
        m_indexOfRefraction(0.0f),
        m_abbeNumber(0.0f),
        m_coatingIor(0.0f),
        m_coatingLambda(0.0f),
        m_texture(0),
        m_textureFT(0)
    {}

    /// Computes the index of refraction corresponding to the paramater 
    /// wavelength, by using the refractive index at the d-line and the abbe 
    /// number of the element.
    ///
    /// \param lambda Desired wavelength, in nanometers.
    float computeIndexOfRefraction(float lambda) const
    {
        // Convert the wavelength to micrometers
        float lambdaMicro = lambda * 0.001f;

        // Compute the coefficients
        float B = ((m_indexOfRefraction - 1.0f) / m_abbeNumber) * 0.52345f;
        float A = m_indexOfRefraction - (B / 0.34522792f);

        // Return the result
        return A + B / (lambdaMicro * lambdaMicro);
    }
    
    /// Returns the type of the optical element.
    ElementType getType() const { return m_type;}
    
    /// Returns the height of the element.
    float getHeight() const { return m_height; }

    /// Returns the thickness of the element.
    float getThickness() const { return m_thickness; }
    
    /// Returns the radius of curvature.
    float getRadiusOfCurvature() const { return m_radiusOfCurvature; }
    
    /// Returns the index of refraction of the element.
    float getIndexOfRefraction() const { return m_indexOfRefraction; }
    
    /// Returns the abbe number of the element's material.
    float getAbbeNumber() const { return m_abbeNumber; }

    /// Returns the target wavelength of the anti reflection coating layer.
    float getCoatingLambda() const { return m_coatingLambda; }

    /// Returns the index of refraction of the anti reflection coating layer.
    float getCoatingIor() const { return m_coatingIor; }
    
    /// Returns the masking texture.
    GLuint getTexture() const { return m_texture; }
    
    /// Returns the Fourier-transformed version of the masking texture.
    GLuint getTextureFT() const { return m_textureFT; }
    
    /// Sets the type of the element.
    void setType(ElementType value) { m_type = value;}
    
    /// Sets the height of the element.
    void setHeight(float value) { m_height = value; }
    
    /// Sets the thickness of the element.
    void setThickness(float value) { m_thickness = value; }

    /// Sets the radius of curvature.
    void setRadiusOfCurvature(float value) { m_radiusOfCurvature = value; }
    
    /// Sets the index of refraction of the element's material.
    void setIndexOfRefraction(float value) { m_indexOfRefraction = value; }
    
    /// Sets the abbe number of the optical element's material.
    void setAbbeNumber(float value) { m_abbeNumber = value; }

    /// Sets the target wavelength the anti reflection coating.
    void setCoatingLambda(float value) { m_coatingLambda = value; }

    /// Sets the index of refraction of the anti reflection coating.
    void setCoatingIor(float value) { m_coatingIor = value; }

    /// Sets the masking texture.
    void setTexture(GLuint value) { m_texture = value; }

    /// Sets the Fourier-transformed version of the masking texture.
    void setTextureFT(GLuint value) { m_textureFT = value; }

private:
    /// Type of the optical element.
    ElementType m_type;
    
    /// Height of the element.
    float m_height;

    /// Thickness of the element.
    float m_thickness;
    
    /// Radius of curvature.
    float m_radiusOfCurvature;
    
    /// Index of refraction of the optical element, at the d-line.
    float m_indexOfRefraction;
    
    /// Abbe number of the element's material.
    float m_abbeNumber;

    /// Target wavelength of the anti reflection coating.
    float m_coatingLambda;

    /// Ior of the anti reflection coating.
    float m_coatingIor;

    /// A masking texture, mainly useful for apertures.
    GLuint m_texture;

    /// FT of the masking texture.
    GLuint m_textureFT;
};

/// Describes an optical system, as a list of ordered optical elements, and 
/// various other system parameters. All attributes values are measured in 
/// millimeters and degrees.
class OpticalSystem
{
public:
    /// Data structure holding the list of elements.
    using ElementList = std::vector<OpticalSystemElement>;

    /// Constructs an empty optical system.
    OpticalSystem():
        OpticalSystem(0.0f, 0.0f, 0.0f, glm::vec2(0.0f))
    {}

    /// Constructs an optical system from the parameters.
    OpticalSystem(float fno, float efl, float fov, glm::vec2 fs):
        OpticalSystem(fno, efl, fov, fs, {})
    {}

    /// Constructs an optical system from the parameters.
    OpticalSystem(float fno, float efl, float fov, glm::vec2 fs, const ElementList& e):
        m_fnumber(fno),
        m_efl(efl),
        m_fov(fov),
        m_filmSize(fs),
        m_elements(e)
    {}

    /// Constructs an optical system from the parameters.
    OpticalSystem(float fno, float efl, float fov, glm::vec2 fs, ElementList&& e):
        m_fnumber(fno),
        m_efl(efl),
        m_fov(fov),
        m_filmSize(fs),
        m_elements(std::move(e))
    {}
    
    /// Returns the name of the optical system.
    const std::string getName() const { return m_name; }

    /// Returns the F-number of the system.
    float getFnumber() const { return m_fnumber; }
    
    /// Returns the effective focal length of the system.
    float getEffectiveFocalLength() const { return m_efl; }
    
    /// Returns the field of view of the system.
    float getFieldOfView() const { return m_fov; }
    
    /// Returns the size of the film used by the optical system.
    glm::vec2 getFilmSize() const { return m_filmSize; }
    
    /// Returns the width of the film used by the optical system.
    float getFilmWidth() const { return m_filmSize.x; }
    
    /// Returns the height of the film used by the optical system.
    float getFilmHeight() const { return m_filmSize.y; }
    
    /// Returns the list of elements that are present in the system.
    const ElementList& getElements() const { return m_elements; }

    /// Returns the number of elements in the system.
    size_t getElementCount() const { return m_elements.size(); }
    
    /// Returns the number of apertures in the system.
    size_t getApertureCount() const
    {
        int result = 0;

        for (auto& lens: m_elements)
        {
            if (lens.getType() == OpticalSystemElement::ElementType::APERTURE_STOP)
            {
                ++result;
            }
        }

        return result;
    }
    
    /// Returns the sensor's distance from the entrace plane.
	float getSensorDistance() const
    {
        return std::accumulate(m_elements.begin(), m_elements.end(), 0.0f, 
            [](float a, const auto& lens)
            {
                return a + lens.getThickness();
            });
    }
    
    /// Returns the maximal height in the system.
	float getTotalHeight() const
    {
        const auto& it = std::max_element(
            m_elements.begin(), m_elements.end(), [](const auto& a, const auto& b)
            {
                return a.getHeight() < b.getHeight();
            });

        return it == m_elements.end() ? 0.0f : it->getHeight();
    }

    /// Returns the effective aperture height (height of the iris projection
    /// on the front element).
    float getEffectiveApertureHeight() const
    {
        return (m_efl / m_fnumber) / 2.0f;
    }

    /// Returns the aspect ratio of the system.
    float getAspectRatio() const { return m_filmSize.x / m_filmSize.y; }
    
    /// Tests whether the parameter ghost can be generated by the optical system.
    bool isValidGhost(const Ghost& ghost) const
    {
        // Check each reflection interface.
        for (int i = 0; i < ghost.getLength(); ++i)
        {
            /// Make sure the index is valid.
            if (ghost[i] >= m_elements.size())
            {
                return false;
            }

            // Make sure it is referring to an actual lens.
            auto type = m_elements[ghost[i]].getType();
            if (type == OpticalSystemElement::ElementType::APERTURE_STOP || 
                type == OpticalSystemElement::ElementType::SENSOR)
            {
                return false;
            }
        }

        // It is a valid ghost path.
        return true;
    }

    /// Enumerates all the ghosts generated by the optical system that match
    /// the parameter filter values.
    GhostList generateGhosts(int maxBounces = 0, bool apertureCross = false)
    {
        // Special treatment for the 'no reflections' case
        if (maxBounces == 0)
        {
            return std::vector<Ghost>{ Ghost{} };
        }

        // Enumerate the apertures
        std::vector<int> interfaces;
        int totalInterfaces = 0;
        
        int prevId = -1;
        for (int i = 0; i < m_elements.size(); ++i)
        {
            auto type = m_elements[i].getType();
            if (type == OpticalSystemElement::ElementType::APERTURE_STOP ||
                type == OpticalSystemElement::ElementType::SENSOR)
            {
                int numInterfaces = i - prevId - 1;

                interfaces.push_back(numInterfaces);
                totalInterfaces += numInterfaces;
                
                prevId = i;
            }
        }
        // Only use a single interface list if aperture crossing is allowed
        if (apertureCross)
        {
            interfaces = { totalInterfaces };
        }

        // The resulting ghost list
        GhostList result;

        // Perform a preliminary counting step to reserve space in advance
        // for large reflection numbers
        if (maxBounces > 4)
        {
            int numGhosts = 0;
            generateGhosts(interfaces, maxBounces, [&](const auto& ghost)
            {
                ++numGhosts;
            });
            result.reserve(numGhosts);
        }

        // Enumerate the ghosts.
        generateGhosts(interfaces, maxBounces, [&](const auto& ghost)
        {
            result.push_back(ghost);
        });

        // We need to remap the indices if interface crossing is allowed
        if (apertureCross)
        {
            // Map between the generated and real indices
            std::vector<int> indexRemap(totalInterfaces);
            
            // How much the index needs to be offset - it is the number of
            // apertures found up to this element
            int offset = 0;

            for (int i = 0; i < totalInterfaces; ++i)
            {
                auto type = m_elements[i + offset].getType();
                while (type == OpticalSystemElement::ElementType::APERTURE_STOP)
                {
                    ++offset;
                }

                indexRemap[i] = i + offset;
            }
        
            // Remap each index of each ghost using the map generated above
            for (auto& ghost: result)
            {
                for (int i = 0; i < ghost.getLength(); ++i)
                {
                    ghost[i] = indexRemap[ghost[i]];
                }
            }
        }

        // Return the result
        return result;
    }

    /// Sets the name of the system.
    void setName(const std::string& value) { m_name = value; }

    /// Sets the F-number of the system.
    void setFnumber(float value) { m_fnumber = value; }
    
    /// Sets the effective focal length of the system.
    void setEffectiveFocalLength(float value) { m_efl = value; }
    
    /// Sets the field of view of the system.
    void setFieldOfView(float value) { m_fov = value; }
    
    /// Sets the film size.
    void setFilmSize(glm::vec2 value) { m_filmSize = value; }
    
    /// Sets the film width.
    void setFilmWidth(float value) { m_filmSize.x = value; }
    
    /// Sets the film height.
    void setFilmHeight(float value) { m_filmSize.y = value; }
    
    /// Sets the element list. This version copies the parameter.
    void setElements(const ElementList& value) { m_elements = value; }

    /// Sets the element list. This version moves the parameter.
    void setElements(ElementList&& value) { m_elements = std::move(value); }

    /// Accesses the ith optical element.
    OpticalSystemElement& operator[](size_t i) { return m_elements[i]; }
    
    /// Accesses the ith optical element.
    const OpticalSystemElement& operator[](size_t i) const { return m_elements[i]; }

    /// Standard iterators to the underlying data.
    using iterator = ElementList::iterator;
    using const_iterator = ElementList::const_iterator;
    using reverse_iterator = ElementList::reverse_iterator;
    using const_reverse_iterator = ElementList::const_reverse_iterator;

    iterator begin() { return m_elements.begin(); }

    const_iterator begin() const { return m_elements.begin(); }

    const_iterator cbegin() const { return m_elements.cbegin(); }

    iterator end() { return m_elements.end(); }

    const_iterator end() const { return m_elements.end(); }

    const_iterator cend() const { return m_elements.cend(); }

    reverse_iterator rbegin() { return m_elements.rbegin(); }

    const_reverse_iterator rbegin() const { return m_elements.rbegin(); }

    const_reverse_iterator crbegin() const { return m_elements.crbegin(); }

    reverse_iterator rend() { return m_elements.rend(); }

    const_reverse_iterator rend() const { return m_elements.rend(); }

    const_reverse_iterator crend() const { return m_elements.crend(); }

private:
    /// Generates all the ghosts that correspond to the parameter set of
    /// interfaces and reflection numbers.
    template<typename FnOut>
    void generateGhosts(const std::vector<int>& interfaces, int numReflections, FnOut output)
    {
        // Holds the indices for the current interface for each interface
        // that participates in the birth of the ghost
        std::vector<int> indices(numReflections);

        // Holds the current ghost interface indices
        std::vector<int> ghost(numReflections);

        // Interface offset, used to manage 'sub-regions' of the optical system
        // as separated by the apertures
        int offset = 0;

        // Process each region separately
        for (auto numInterfaces: interfaces)
        {
            // Init the indices for the current generation
            for (int i = 0; i < numReflections; ++i)
            {
                indices[i] = (i + 1) % 2;
            }

            // Enumerate each ghost
            while (indices[0] != numInterfaces)
            {
                // Generate the ghost indices
                for (int i = 0; i < numReflections; ++i)
                {
                    ghost[i] = offset + indices[i];
                }
                
                // Store the ghost
                output(ghost);
                
                // Increment each interface from back to front, until we reach
                // the first valid index, then note its position in the list
                int startIndex = numReflections;
                for (int i = numReflections - 1; i >= 0; --i, --startIndex)
                {
                    // Increment the index
                    indices[i] = indices[i] + 1;
                    
                    // First interfaces - these can go until the last element
                    if (i % 2 == 0 && (i == 0 || indices[i] < numInterfaces - 1))
                        break;
                    
                    // Second interfaces - thes can only go until the preceding
                    // interface index
                    if (i % 2 == 1 && (i == 0 || indices[i] < indices[i - 1]))
                        break;
                }
                
                // Re-initialize the indices that became invalid
                for (int i = startIndex; i < numReflections; ++i)
                {
                    // First interfaces - 
                    if (i % 2 == 0)
                    {
                        indices[i] = indices[i - 1] + 1;
                    }
                    
                    // Second interfaces - these start from the back
                    if (i % 2 == 1)
                    {
                        indices[i] = 0;
                    }
                }
            }
            
            // Increment the offset
            offset = offset + numInterfaces + 1;
        }
    }

    /// Name of the optical system.
    std::string m_name;

    /// The F-number of the system.
    float m_fnumber;
    
    /// The effective focal length of the system.
    float m_efl;
    
    /// The field of view of the system.
    float m_fov;
    
    /// Size of the film.
    glm::vec2 m_filmSize;
    
    /// The elements that are present in the system.
    ElementList m_elements;
};

}