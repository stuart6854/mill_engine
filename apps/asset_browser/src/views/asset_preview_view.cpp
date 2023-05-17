#include "asset_preview_view.hpp"

#include <mill/mill.hpp>

#include <imgui.h>

namespace mill::asset_browser
{
    void AssetPreviewView::init(u64 scene_view_id)
    {
        m_sceneViewId = scene_view_id;

        rhi::ResourceSetDescription set_desc{
            .bindings = {   
                { rhi::ResourceType::eTexture, 1, rhi::ShaderStage::eFragment },    
            },
        };
        m_resourceSetId = rhi::create_resource_set(set_desc);
        rhi::bind_view_to_resource_set(m_resourceSetId, 0, m_sceneViewId);

        m_entity = Engine::get()->get_scene_manager()->get_active_scene().create_entity();
        m_entity.add_component<StaticMeshComponent>();
    }

    void AssetPreviewView::render()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Preview"))
        {
            auto ContentSize = ImGui::GetContentRegionAvail();
            // rhi::reset_view(m_sceneViewId, CAST_U32(ContentSize.x), CAST_U32(ContentSize.y));

            ImGui::Image(&m_resourceSetId, ContentSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

}
