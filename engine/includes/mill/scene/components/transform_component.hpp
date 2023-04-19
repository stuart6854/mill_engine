#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace mill
{
    struct TransformComponent
    {
        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;

        void set_position(const glm::vec3& position);
        void set_rotation(const glm::quat& rotation);
        void set_scale(const glm::vec3& scale);

        /* Getters */

        auto get_position() const -> const glm::vec3&;
        auto get_rotation() const -> const glm::quat&;
        auto get_scale() const -> const glm::vec3&;

        auto get_transform() -> const glm::mat4&;

    private:
        void update_transform();

    private:
        glm::vec3 m_position{};
        glm::quat m_rotation{};
        glm::vec3 m_scale{ 1, 1, 1 };

        bool m_dirty{};
        glm::mat4 m_transform{ 1.0f };
    };
}