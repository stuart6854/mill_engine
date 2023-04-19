#include "mill/scene/components/transform_component.hpp"

#include <glm/gtc/quaternion.hpp>

namespace mill
{
    void TransformComponent::set_position(const glm::vec3& position)
    {
        m_position = position;

        m_dirty = true;
    }

    void TransformComponent::set_rotation(const glm::quat& rotation)
    {
        m_rotation = rotation;

        m_dirty = true;
    }

    void TransformComponent::set_scale(const glm::vec3& scale)
    {
        m_scale = scale;

        m_dirty = true;
    }

    auto TransformComponent::get_position() const -> const glm::vec3&
    {
        return m_position;
    }

    auto TransformComponent::get_rotation() const -> const glm::quat&
    {
        return m_rotation;
    }

    auto TransformComponent::get_scale() const -> const glm::vec3&
    {
        return m_scale;
    }

    auto TransformComponent::get_transform() -> const glm::mat4&
    {
        if (m_dirty)
            update_transform();

        return m_transform;
    }

    void TransformComponent::update_transform()
    {
        auto translate = glm::translate(glm::mat4(1.0f), m_position);
        auto rotate = glm::mat4_cast(m_rotation);
        auto scale = glm::scale(glm::mat4(1.0f), m_scale);
        m_transform = translate * rotate * scale;

        m_dirty = false;
    }

}
