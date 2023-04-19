#include "mill/scene/scene.hpp"

#include "mill/core/engine.hpp"
#include "mill/resources/resource_manager.hpp"
#include "mill/scene/entity.hpp"
#include "mill/scene/components/transform_component.hpp"
#include "mill/scene/components/static_mesh_component.hpp"
#include "mill/utility/random.hpp"

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace mill
{
    Scene::Scene() = default;

    Scene::~Scene() = default;

    void Scene::on_load()
    {
        auto* resources = Engine::get()->get_resources();

        {
            auto entity = create_entity();
            auto& transform = entity.add_component<TransformComponent>();
            transform.set_position({ 0, 0, 0 });

            const auto rand_yaw = random::random_f32_range(-180.0f, 180.0f);
            const glm::vec3 euler_angles{ 0, rand_yaw, 0 };

            const auto rotation = glm::quat(euler_angles);
            transform.set_rotation(rotation);

            auto& static_mesh = entity.add_component<StaticMeshComponent>();
            static_mesh.staticMesh = resources->get_handle(1998, true);
        }
        {
            auto entity = create_entity();
            auto& transform = entity.add_component<TransformComponent>();
            transform.set_position({ 0, 5, 0 });

            const auto rand_yaw = random::random_f32_range(-180.0f, 180.0f);
            const glm::vec3 euler_angles{ 0, rand_yaw, 0 };

            const auto rotation = glm::quat(euler_angles);
            transform.set_rotation(rotation);

            auto& static_mesh = entity.add_component<StaticMeshComponent>();
            static_mesh.staticMesh = resources->get_handle(1998, true);
        }
        {
            auto entity = create_entity();
            auto& transform = entity.add_component<TransformComponent>();
            transform.set_position({ 0, 0, 5 });

            const auto rand_yaw = random::random_f32_range(-180.0f, 180.0f);
            const glm::vec3 euler_angles{ 0, rand_yaw, 0 };

            const auto rotation = glm::quat(euler_angles);
            transform.set_rotation(rotation);

            auto& static_mesh = entity.add_component<StaticMeshComponent>();
            static_mesh.staticMesh = resources->get_handle(1998, true);
        }
        {
            auto entity = create_entity();
            auto& transform = entity.add_component<TransformComponent>();
            transform.set_position({ 5, 0, 0 });

            const auto rand_yaw = random::random_f32_range(-180.0f, 180.0f);
            const glm::vec3 euler_angles{ 0, rand_yaw, 0 };

            const auto rotation = glm::quat(euler_angles);
            transform.set_rotation(rotation);

            auto& static_mesh = entity.add_component<StaticMeshComponent>();
            static_mesh.staticMesh = resources->get_handle(1998, true);
        }
    }

    void Scene::on_unload() {}

    void Scene::tick(f32 delta_time)
    {
        auto view = m_registry.view<TransformComponent>();
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);

            const auto euler_angles = glm::vec3(0, 10.0f, 0) * delta_time;
            const auto rot_amount = glm::quat(glm::radians(euler_angles));

            auto new_rotation = transform.get_rotation() * rot_amount;
            transform.set_rotation(new_rotation);
        }
    }

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
