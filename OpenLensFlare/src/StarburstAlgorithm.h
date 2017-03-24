#pragma once

#include "Dependencies.h"
#include "OpticalSystem.h"
#include "LightSource.h"

namespace OLEF
{

/// Represents a physically-based starburst rendering algorithm.
class StarburstAlgorithm
{
public:
    /// Polymorphic destructor.
    virtual ~StarburstAlgorithm() {}

    /// Renders the starburst corresponding to the parameter light source.
    virtual void renderStarburst(const LightSource& light) = 0;

    /// Returns a pointer to the optical system that generates the starburst.
    virtual OpticalSystem* getOpticalSystem() const = 0;
};

}