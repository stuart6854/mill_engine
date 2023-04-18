#include "mill/scene/scene.hpp"

#include "mill/scene/entity.hpp"

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace mill
{
    Scene::Scene() {}

    Scene::~Scene() = default;

    void Scene::on_load() {}

    void Scene::on_unload() {}

    void Scene::tick(f32 /*delta_time*/) {}

    auto Scene::create_entity() -> Entity
    {
        Entity entity(*this, m_registry.create());
        return entity;
    }

    void Scene::destroy_entity(Entity& entity)
    {
        m_registry.destroy(static_cast<entt::entity>(entity));
    }

    auto Scene::get_registry() -> entt::registry&
    {
        return m_registry;
    }

}
