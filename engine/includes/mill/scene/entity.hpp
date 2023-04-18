#pragma once

#include "scene.hpp"

#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>

namespace mill
{
    class Scene;

    class Entity
    {
    public:
        Entity();
        Entity(Scene& scene, entt::entity entity);
        Entity(const Entity&) = default;
        Entity(Entity&&) = default;
        ~Entity();

        template <typename T>
        bool has_component() const;

        template <typename T>
        auto add_component() -> T&;

        template <typename T>
        void remove_component();

        template <typename T>
        bool get_component(T&& out_component);

        /* Getters */

        auto get_scene() const -> const Scene&;

        /* Operators */

        operator bool() const;

        bool operator==(const Entity& rhs) const;
        bool operator!=(const Entity& rhs) const;

        operator u64() const;
        operator entt::entity() const;

    private:
        Scene* m_scene{};
        entt::entity m_entity{};
    };

    template <typename T>
    bool mill::Entity::has_component() const
    {
        return m_scene->get_registry().all_of<T>(m_entity);
    }

    template <typename T>
    auto mill::Entity::add_component() -> T&
    {
        m_scene->get_registry().emplace<T>(m_entity);
    }

    template <typename T>
    void mill::Entity::remove_component()
    {
        m_scene->get_registry().remove<T>(m_entity);
    }

    template <typename T>
    bool mill::Entity::get_component(T&& out_component)
    {
        auto* comp = m_scene->get_registry().try_get<T>(m_entity);
        if (comp)
        {
            out_component = *comp;
            return true;
        }
        return false;
    }

}