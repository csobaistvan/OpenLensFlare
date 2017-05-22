#pragma once

// Standard headers
#include <utility>   // For std::move.
#include <cassert>   // For parameter validations.
#include <string>    // For string handling.
#include <iostream>  // For serialization of certain objects
#include <array>     // For statically sized arrays.
#include <vector>    // For dynamic arrays.
#include <map>       // For mapping data to certain ghosts.
#include <numeric>   // For std algorithms.
#include <algorithm> // For std algorithms.

// GLEW
#define GLEW_STATIC
#include "glew.h"

// GLM
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_transform.hpp"