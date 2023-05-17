#include "mill/scene/entity.hpp"

namespace mill
{
    Entity::Entity() = default;

    Entity::Entity(Scene& scene, entt::entity entity) : m_scene(&scene), m_entity(entity) {}

    Entity::Entity(const Entity& other) : m_scene(other.m_scene), m_entity(other.m_entity) {}

    Entity::Entity(Entity&& other) noexcept
    {
        std::swap(m_scene, other.m_scene);
        std::swap(m_entity, other.m_entity);
    }

    Entity::~Entity() = default;

    auto Entity::get_scene() const -> const Scene&
    {
        return *m_scene;
    }

    auto Entity::operator=(const Entity& rhs) -> Entity&
    {
        m_scene = rhs.m_scene;
        m_entity = rhs.m_entity;
        return *this;
    }

    auto Entity::operator=(Entity&& rhs) noexcept -> Entity&
    {
        std::swap(m_scene, rhs.m_scene);
        std::swap(m_entity, rhs.m_entity);
        return *this;
    }

    Entity::operator bool() const
    {
        return m_scene != nullptr && CAST_BOOL(m_entity);
    }

    Entity::operator u64() const
    {
        return CAST_U64(m_entity);
    }

    bool Entity::operator==(const Entity& rhs) const
    {
        return m_scene == rhs.m_scene && m_entity == rhs.m_entity;
    }

    bool Entity::operator!=(const Entity& rhs) const
    {
        return !(*this == rhs);
    }

    Entity::operator entt::entity() const
    {
        return m_entity;
    }

}
