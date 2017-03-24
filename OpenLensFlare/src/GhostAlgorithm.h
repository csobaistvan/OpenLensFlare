#pragma once

#include "Dependencies.h"
#include "OpticalSystem.h"
#include "GhostList.h"
#include "LightSource.h"

namespace OLEF
{

/// Represents a physically-based ghost rendering algorithm.
class GhostAlgorithm
{
public:
    /// Polymorphic destructor.
    virtual ~GhostAlgorithm() {}

    /// Renders the ghosts corresponding to the parameter light source.
    virtual void renderGhosts(const LightSource& light, int first, int count) = 0;

    /// Returns the list of ghosts this object can render.
    virtual const GhostList& getGhostList() const = 0;

    /// Convenience function. Renders all ghosts, based on the virtual functions
    /// declared above.
    void renderAllGhosts(const LightSource& light)
    {
        renderGhosts(light, 0, (int) getGhostList().getGhosts().size());
    }
};

}