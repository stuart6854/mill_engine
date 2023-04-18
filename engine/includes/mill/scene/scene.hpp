#pragma once

#include "mill/core/base.hpp"

#include <entt/entity/registry.hpp>

namespace mill
{
    class Entity;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void on_load();
        void on_unload();

        void tick(f32 delta_time);

        auto create_entity() -> Entity;
        void destroy_entity(Entity& entity);

        /* Getters */

        auto get_registry() -> entt::registry&;

    private:
        entt::registry m_registry{};
    };
}