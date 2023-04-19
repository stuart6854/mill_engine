#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

namespace mill
{
    struct StaticVertex
    {
        glm::vec3 position{};
        glm::vec2 texCoord{};
        glm::vec3 color{};
    };
}