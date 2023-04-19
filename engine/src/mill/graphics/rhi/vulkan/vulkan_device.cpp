#include "vulkan_device.hpp"

#include "mill/core/base.hpp"
#include "mill/core/debug.hpp"
#include "rhi_resource_vulkan.hpp"
#include "vulkan_device.hpp"
#include "vulkan_screen.hpp"
#include "vulkan_context.hpp"
#include "vulkan_view.hpp"
#include "vulkan_helpers.hpp"
#include "resources/descriptor_set_layout.hpp"
#include "resources/pipeline_layout.hpp"
#include "resources/pipeline_module_vertex_input.hpp"
#include "resources/pipeline_module_pre_rasterisation.hpp"
#include "resources/pipeline_module_fragment_stage.hpp"
#include "resources/pipeline_module_fragment_output.hpp"
#include "resources/pipeline.hpp"
#include "resources/descriptor_set.hpp"
#include "resources/buffer.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <spirv_cross.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mill::rhi
{
    constexpr auto g_VulkanAPIVersion = VK_API_VERSION_1_3;
    constexpr bool g_PipelineLinkTimeOptimisation = true;  // Enable for better runtime performance (costs slower compile/linking time)

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                          VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                          void* user_data);

    DeviceVulkan::DeviceVulkan() = default;

    DeviceVulkan::~DeviceVulkan()
    {
        shutdown();
    }

    bool DeviceVulkan::initialise()
    {
        if (!init_instance())
            return false;

        if (!init_physical_device())
            return false;

        if (!init_device())
            return false;

        // Allocator
        {
            vma::AllocatorCreateInfo alloc_info{};
            alloc_info.setInstance(m_instance.get());
            alloc_info.setPhysicalDevice(m_physicalDevice);
            alloc_info.setDevice(m_device.get());
            alloc_info.setVulkanApiVersion(g_VulkanAPIVersion);
            m_allocator = vma::createAllocatorUnique(alloc_info);
        }

        // Descriptor Pool
        {
            std::vector<vk::DescriptorPoolSize> pool_sizes{
                { vk::DescriptorType::eUniformBuffer, 100 },
                { vk::DescriptorType::eStorageBuffer, 100 },
                { vk::DescriptorType::eCombinedImageSampler, 100 },
            };

            vk::DescriptorPoolCreateInfo pool_info{};
            pool_info.setMaxSets(100);
            pool_info.setPoolSizes(pool_sizes);
            pool_info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
            m_descriptorPool = m_device->createDescriptorPoolUnique(pool_info);
        }

        return true;
    }

    void DeviceVulkan::shutdown()
    {
        if (!m_instance && !m_device)
            return;

        wait_idle();
    }

    void DeviceVulkan::wait_idle() const
    {
        ASSERT(m_device);
        m_device->waitIdle();
    }

#pragma region Resources

    void DeviceVulkan::create_screen(u64 screen_id, void* window_handle)
    {
        ASSERT(m_screens.contains(screen_id) == false);

        m_screens[screen_id] = CreateOwned<ScreenVulkan>(*this, screen_id, window_handle);
        m_allScreens.push_back(m_screens[screen_id].get());
    }

    void DeviceVulkan::destroy_screen(u64 screen_id)
    {
        if (!m_screens.contains(screen_id))
            return;

        auto* screen = m_screens.at(screen_id).get();
        std::erase(m_allScreens, screen);

        m_screens.erase(screen_id);
    }

    auto DeviceVulkan::get_screen(u64 screen_id) const -> ScreenVulkan*
    {
        if (!m_screens.contains(screen_id))
            return nullptr;

        const auto& screen = m_screens.at(screen_id);
        return screen.get();
    }

    auto DeviceVulkan::get_all_screens() const -> const std::vector<ScreenVulkan*>&
    {
        return m_allScreens;
    }

    void DeviceVulkan::create_context(u64 context_id)
    {
        ASSERT(m_contexts.contains(context_id) == false);

        m_contexts[context_id] = CreateOwned<ContextVulkan>(*this);
        m_allContexts.push_back(m_contexts.at(context_id).get());
    }

    void DeviceVulkan::destroy_context(u64 context_id)
    {
        if (!m_contexts.contains(context_id))
            return;

        auto* context = m_contexts.at(context_id).get();
        std::erase(m_allContexts, context);

        m_contexts.erase(context_id);
    }

    auto DeviceVulkan::get_context(u64 context_id) const -> ContextVulkan*
    {
        if (!m_contexts.contains(context_id))
            return nullptr;

        const auto& context = m_contexts.at(context_id);
        return context.get();
    }

    auto DeviceVulkan::get_all_contexts() const -> const std::vector<ContextVulkan*>&
    {
        return m_allContexts;
    }

    void DeviceVulkan::create_view(u64 view_id)
    {
        ASSERT(m_views.contains(view_id) == false);

        m_views[view_id] = CreateOwned<ViewVulkan>(*this);
    }

    void DeviceVulkan::destroy_view(u64 view_id)
    {
        if (!m_views.contains(view_id))
            return;

        m_views.erase(view_id);
    }

    auto DeviceVulkan::get_view(u64 view_id) const -> ViewVulkan*
    {
        if (!m_views.contains(view_id))
            return nullptr;

        const auto& view = m_views.at(view_id);
        return view.get();
    }

    /* Resource Sets */

    auto DeviceVulkan::get_or_create_resource_set_layout(const ResourceSetDescriptionVulkan& description) -> Shared<DescriptorSetLayout>
    {
        auto layout = CreateShared<DescriptorSetLayout>(m_device.get());

        for (u32 i = 0; i < description.bindings.size(); ++i)
        {
            const auto& binding = description.bindings.at(i);
            switch (binding.type)
            {
                case vk::DescriptorType::eUniformBuffer: layout->add_uniform_buffer(i); break;
                case vk::DescriptorType::eCombinedImageSampler: layout->add_sampled_image(i); break;
                default: ASSERT(("Unknown DescriptorType!", false)); break;
            }
        }

        const auto layout_hash = layout->get_hash();
        ASSERT(layout_hash);

        if (m_descriptorSetLayouts.contains(layout_hash))
            return m_descriptorSetLayouts.at(layout_hash);

        m_descriptorSetLayouts[layout_hash] = layout;

        layout->build();

        LOG_DEBUG("DeviceVulkan - Descriptor Set Layout has been created: {}", layout_hash);

        return layout;
    }

    auto DeviceVulkan::create_resource_set(const ResourceSetDescriptionVulkan& description) -> u64
    {
        auto layout = get_or_create_resource_set_layout(description);
        ASSERT(layout);

        const auto id = m_nextResourceSetId;
        ++m_nextResourceSetId;

        m_descriptorSets[id] = CreateShared<DescriptorSet>(*this, layout, description.buffered);

        LOG_DEBUG("DeviceVulkan - ResourceSet has been created: {}", id);

        return id;
    }

    /* Pipelines */

    auto DeviceVulkan::get_or_create_pipeline_vertex_input_module(const PipelineVertexInputStateVulkan& state)
        -> Shared<PipelineModuleVertexInput>
    {
        auto module = CreateShared<PipelineModuleVertexInput>(*this);
        module->set_input_attributes(state.attributes);
        module->set_topology(state.topology);

        const auto module_hash = module->get_hash();
        ASSERT(module_hash);

        if (m_vertexInputPipelineModules.contains(module_hash))
            return m_vertexInputPipelineModules.at(module_hash);

        m_vertexInputPipelineModules[module_hash] = module;

        module->build();

        LOG_DEBUG("DeviceVulkan - Pipeline Module for <VertexInput> has been created: {}", module_hash);

        return module;
    }

    auto DeviceVulkan::get_or_create_pipeline_pre_rasterisation_module(const PipelinePreRasterisationStateVulkan& state)
        -> Shared<PipelineModulePreRasterisation>
    {
        ASSERT(!state.vertexSpirv.empty());

        auto module = CreateShared<PipelineModulePreRasterisation>(*this);

        auto layout = reflect_shader_stage_pipeline_layout(state.vertexSpirv, vk::ShaderStageFlagBits::eVertex);
        ASSERT(layout);

        module->set_layout(layout);
        module->set_vertex_spirv(state.vertexSpirv);
        module->set_polygon_mode(state.fillMode);
        module->set_cull_mode(state.cullMode);
        module->set_front_face(state.frontFace);
        module->set_line_width(state.lineWidth);

        const auto module_hash = module->get_hash();
        ASSERT(module_hash);

        if (m_preRasterisationPipelineModules.contains(module_hash))
            return m_preRasterisationPipelineModules.at(module_hash);

        m_preRasterisationPipelineModules[module_hash] = module;

        module->build();

        LOG_DEBUG("DeviceVulkan - Pipeline Module for <PreRasterisation> has been created: {}", module_hash);

        return module;
    }

    auto DeviceVulkan::get_or_create_pipeline_fragment_stage_module(const PipelineFragmentStageStateVulkan& state)
        -> Shared<PipelineModuleFragmentStage>
    {
        ASSERT(!state.fragmentSpirv.empty());

        auto module = CreateShared<PipelineModuleFragmentStage>(*this);

        auto layout = reflect_shader_stage_pipeline_layout(state.fragmentSpirv, vk::ShaderStageFlagBits::eFragment);
        ASSERT(layout);

        module->set_layout(layout);
        module->set_fragment_spirv(state.fragmentSpirv);
        module->set_depth_test(state.depthTest);
        module->set_stencil_test(state.stencilTest);

        const auto module_hash = module->get_hash();
        ASSERT(module_hash);

        if (m_fragmentStagePipelineModules.contains(module_hash))
            return m_fragmentStagePipelineModules.at(module_hash);

        m_fragmentStagePipelineModules[module_hash] = module;

        module->build();

        LOG_DEBUG("DeviceVulkan - Pipeline Module for <FragmentStage> has been created: {}", module_hash);

        return module;
    }

    auto DeviceVulkan::get_or_create_pipeline_fragment_output_module(const PipelineFragmentOutputStateVulkan& state)
        -> Shared<PipelineModuleFragmentOutput>
    {
        auto module = CreateShared<PipelineModuleFragmentOutput>(*this);

        module->set_color_blend(state.colorBlend);

        const auto module_hash = module->get_hash();
        ASSERT(module_hash);

        if (m_fragmentOutputPipelineModules.contains(module_hash))
            return m_fragmentOutputPipelineModules.at(module_hash);

        m_fragmentOutputPipelineModules[module_hash] = module;

        module->build();

        LOG_DEBUG("DeviceVulkan - Pipeline Module for <FragmentOutput> has been created: {}", module_hash);

        return module;
    }

    auto DeviceVulkan::get_or_create_pipeline(const std::vector<Shared<PipelineModule>>& modules) -> Shared<Pipeline>
    {
        auto pipeline = CreateShared<Pipeline>(*this);

        // Merge pipeline layouts
        auto merged_layout = CreateShared<PipelineLayout>(m_device.get());
        for (const auto& module : modules)
        {
            if (module->get_layout() != nullptr)
                merged_layout = merge_pipeline_layouts(*merged_layout, *module->get_layout());

            pipeline->add_module(module);
        }

        // Cache pipeline layout or Get existing
        const hasht pipeline_layout_hash = merged_layout->get_hash();
        if (m_pipelineLayouts.contains(pipeline_layout_hash))
        {
            merged_layout = m_pipelineLayouts.at(pipeline_layout_hash);
        }
        else
        {
            merged_layout->build();
            m_pipelineLayouts[pipeline_layout_hash] = merged_layout;
        }

        pipeline->set_layout(merged_layout);

        const hasht pipeline_hash = pipeline->get_hash();
        ASSERT(pipeline_hash);

        if (m_compiledPipelines.contains(pipeline_hash))
            return m_compiledPipelines.at(pipeline_hash);

        m_compiledPipelines[pipeline_hash] = pipeline;

        pipeline->build();

        LOG_DEBUG("DeviceVulkan - Pipeline ({} modules) has been created: {}", modules.size(), pipeline_hash);

        return pipeline;
    }

    auto DeviceVulkan::get_pipeline(hasht pipeline_hash) -> Shared<Pipeline>
    {
        ASSERT(m_compiledPipelines.contains(pipeline_hash));
        return m_compiledPipelines.at(pipeline_hash);
    }

    /* Buffers */

    auto DeviceVulkan::get_buffer(u64 buffer_id) -> const Buffer&
    {
        ASSERT(m_buffers.contains(buffer_id));

        const auto& buffer = m_buffers.at(buffer_id);
        return *buffer;
    }

    auto DeviceVulkan::create_buffer(const BufferDescriptionVulkan& description) -> u64
    {
        const auto buffer_id = m_nextBufferId;
        ++m_nextBufferId;

        m_buffers[buffer_id] = CreateOwned<Buffer>(*this);

        auto& buffer = m_buffers[buffer_id];
        buffer->set_size(description.size);
        buffer->set_usage(description.usage);
        buffer->set_memory_usage(description.memoryUsage);
        buffer->set_alloc_flags(description.allocFlags);

        buffer->build();

        LOG_DEBUG("DeviceVulkan - Buffer has been created: id={}, size={}B", buffer_id, description.size);

        return buffer_id;
    }

    void DeviceVulkan::write_buffer(u64 buffer_id, u64 offset, u64 size, const void* data)
    {
        ASSERT(m_buffers.contains(buffer_id));

        auto& buffer = m_buffers.at(buffer_id);

        bool is_host_visible = CAST_BOOL(buffer->get_alloc_flags() & vma::AllocationCreateFlagBits::eHostAccessSequentialWrite);
        if (is_host_visible)
        {
            void* mapped = m_allocator->mapMemory(buffer->get_allocation());
            ASSERT(mapped);

            u8* offset_mapped = static_cast<u8*>(mapped) + offset;
            ASSERT(offset_mapped);

            std::memcpy(offset_mapped, data, size);

            m_allocator->unmapMemory(buffer->get_allocation());

            return;
        }

        LOG_WARN("DeviceVulkan - Writing to non-HostVisible buffers is not yet supported.");

        // #TODO: Staging buffer
    }

#pragma endregion

    auto DeviceVulkan::get_instance() -> vk::Instance&
    {
        return m_instance.get();
    }

    auto DeviceVulkan::get_physical_device() -> vk::PhysicalDevice&
    {
        return m_physicalDevice;
    }

    auto DeviceVulkan::get_device() -> vk::Device&
    {
        return m_device.get();
    }

    auto DeviceVulkan::get_graphics_queue_family() -> i32
    {
        return m_graphicsQueueFamily;
    }

    auto DeviceVulkan::get_compute_queue_family() -> i32
    {
        return m_computeQueueFamily;
    }

    auto DeviceVulkan::get_transfer_queue_family() -> i32
    {
        return m_transferQueueFamily;
    }

    auto DeviceVulkan::get_graphics_queue() -> vk::Queue&
    {
        return m_graphicsQueue;
    }

    auto DeviceVulkan::get_compute_queue() -> vk::Queue&
    {
        return m_computeQueue;
    }

    auto DeviceVulkan::get_transfer_queue() -> vk::Queue&
    {
        return m_transferQueue;
    }

    auto DeviceVulkan::get_allocator() -> vma::Allocator&
    {
        return m_allocator.get();
    }

    auto DeviceVulkan::get_descriptor_pool() -> vk::DescriptorPool&
    {
        return m_descriptorPool.get();
    }

    bool DeviceVulkan::init_instance()
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

        vk::ApplicationInfo vk_app_info{};
        vk_app_info.setPEngineName("Mill_Engine");
        vk_app_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
        vk_app_info.setPApplicationName("mill_engine_app");
        vk_app_info.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
        vk_app_info.setApiVersion(g_VulkanAPIVersion);

        std::vector instance_exts
        {
#if MILL_DEBUG
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif

                VK_KHR_SURFACE_EXTENSION_NAME,

#if MILL_WINDOWS
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
        };

        for (const auto& ext : instance_exts)
        {
            if (!vulkan::is_instance_ext_supported(ext))
            {
                LOG_ERROR("DeviceVulkan - The instance extension '{}' is not supported!", ext);
            }
        }

        std::vector instance_layers
        {
#if MILL_DEBUG
            "VK_LAYER_KHRONOS_validation",
#endif

                "VK_LAYER_KHRONOS_synchronization2",
        };

        for (const auto& layer : instance_layers)
        {
            if (!vulkan::is_instance_layer_supported(layer))
            {
                LOG_ERROR("DeviceVulkan - The instance layer '{}' is not supported!", layer);
            }
        }

        vk::InstanceCreateInfo instance_info{};
        instance_info.setPApplicationInfo(&vk_app_info);
        instance_info.setPEnabledExtensionNames(instance_exts);
        instance_info.setPEnabledLayerNames(instance_layers);

#if MILL_DEBUG
        vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
        debug_messenger_info.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning);
        debug_messenger_info.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
        debug_messenger_info.setPfnUserCallback(&debug_message_callback);
        instance_info.setPNext(&debug_messenger_info);
#endif
        m_instance = vk::createInstanceUnique(instance_info);
        if (!m_instance)
        {
            LOG_ERROR("DeviceVulkan - Failed to create instance!");
            return false;
        }

        VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_instance);

#if MILL_DEBUG
        m_debugMessenger = m_instance->createDebugUtilsMessengerEXTUnique(debug_messenger_info);
#endif

        LOG_DEBUG("DeviceVulkan - The Vulkan instance has been created.");
        LOG_DEBUG("DeviceVulkan - The following extensions were enabled:");
        for (const auto& ext : instance_exts)
        {
            LOG_DEBUG("\t{}", ext);
        }
        LOG_DEBUG("DeviceVulkan - The following layers were enabled:");
        for (const auto& layer : instance_layers)
        {
            LOG_DEBUG("\t{}", layer);
        }

        return true;
    }

    bool DeviceVulkan::init_physical_device()
    {
        m_physicalDevice = vulkan::get_best_dedicated_gpu(*m_instance);
        if (!m_physicalDevice)
        {
            LOG_ERROR("DeviceVulkan - Failed to select a physical device!");
            return false;
        }

        auto gpu_props = m_physicalDevice.getProperties();
        LOG_INFO(
            "{}, Driver {}.{}", gpu_props.deviceName, VK_VERSION_MAJOR(gpu_props.driverVersion), VK_VERSION_MINOR(gpu_props.driverVersion));

        return true;
    }

    bool DeviceVulkan::init_device()
    {
        std::vector<const char*> device_exts{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
            VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME,
        };

        for (const auto& ext : device_exts)
        {
            if (!vulkan::is_device_ext_supported(m_physicalDevice, ext))
            {
                LOG_ERROR("DeviceVulkan - The device extension '{}' is not supported!", ext);
            }
        }

        vk::PhysicalDeviceFeatures features{};
        features.setFillModeNonSolid(true);
        features.setWideLines(true);

        vk::PhysicalDeviceSynchronization2Features sync2_features{};
        sync2_features.setSynchronization2(true);

        vk::PhysicalDeviceDynamicRenderingFeatures dyn_rendering_features{};
        dyn_rendering_features.setDynamicRendering(true);
        dyn_rendering_features.setPNext(&sync2_features);

        vk::PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT graphics_pipeline_library_features{};
        graphics_pipeline_library_features.setGraphicsPipelineLibrary(true);
        graphics_pipeline_library_features.setPNext(&dyn_rendering_features);

        // #TODO: Check if descriptor indexing is supported
        vk::PhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{};
        descriptor_indexing_features.setDescriptorBindingPartiallyBound(true);
        descriptor_indexing_features.setRuntimeDescriptorArray(true);
        descriptor_indexing_features.setPNext(&graphics_pipeline_library_features);

        m_graphicsQueueFamily = vulkan::find_graphics_queue_family(m_physicalDevice);
        m_transferQueueFamily = vulkan::find_transfer_queue_family(m_physicalDevice);

        std::vector<vk::DeviceQueueCreateInfo> queue_infos{};
        const f32 queue_priority = 1.0f;
        if (m_graphicsQueueFamily != -1)
        {
            auto& queue_info = queue_infos.emplace_back();
            queue_info.setQueueFamilyIndex(m_graphicsQueueFamily);
            queue_info.setQueueCount(1);
            queue_info.setQueuePriorities(queue_priority);
        }
        if (m_transferQueueFamily != -1)
        {
            auto& queue_info = queue_infos.emplace_back();
            queue_info.setQueueFamilyIndex(m_transferQueueFamily);
            queue_info.setQueueCount(1);
            queue_info.setQueuePriorities(queue_priority);
        }

        vk::DeviceCreateInfo device_info{};
        device_info.setPEnabledExtensionNames(device_exts);
        device_info.setPEnabledFeatures(&features);
        device_info.setQueueCreateInfos(queue_infos);
        device_info.setPNext(&descriptor_indexing_features);

        m_device = m_physicalDevice.createDeviceUnique(device_info);
        if (!m_device)
        {
            LOG_ERROR("DeviceVulkan - Failed to create device!");
            return false;
        }

        LOG_DEBUG("DeviceVulkan - The Vulkan device has been created.");
        if (!device_exts.empty())
        {
            LOG_DEBUG("DeviceVulkan - The following extensions were enabled:");
            for (const auto& ext : device_exts)
            {
                LOG_DEBUG("\t{}", ext);
            }
        }

        VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_device);

        if (m_graphicsQueueFamily != -1)
        {
            m_graphicsQueue = m_device->getQueue(m_graphicsQueueFamily, 0);
            SET_VK_OBJECT_NAME(*m_device, VkQueue, m_graphicsQueue, "Main Queue (Graphics)");
        }
        if (m_transferQueueFamily != -1)
        {
            m_transferQueue = m_device->getQueue(m_transferQueueFamily, 0);
            SET_VK_OBJECT_NAME(*m_device, VkQueue, m_transferQueue, "Main Queue (Transfer)");
        }

        return true;
    }

    auto DeviceVulkan::reflect_shader_stage_pipeline_layout(const std::vector<u32>& spirv, vk::ShaderStageFlagBits shader_stage)
        -> Shared<PipelineLayout>
    {
        auto pipeline_layout = CreateShared<PipelineLayout>(m_device.get(), vk::PipelineLayoutCreateFlagBits::eIndependentSetsEXT);

        spirv_cross::Compiler compiler(spirv);
        auto shader_resources = compiler.get_shader_resources();

        for (const auto& const_range : shader_resources.push_constant_buffers)
        {
            const auto& type = compiler.get_type(const_range.base_type_id);
            const u32 size = CAST_U32(compiler.get_declared_struct_size(type));

            pipeline_layout->add_push_constant_buffer(0, size, shader_stage);
        }

#if 0
        for (const auto& ubo : shader_resources.uniform_buffers)
        {
            const u32 set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
            const u32 binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);

            pipeline_layout_state.add_binding(set, binding, shader_stage, ResourceType::eUniformBuffer);
        }
#endif

        const auto pipeline_layout_hash = pipeline_layout->get_hash();
        if (m_pipelineLayouts.contains(pipeline_layout_hash))
            return m_pipelineLayouts.at(pipeline_layout_hash);

        m_pipelineLayouts[pipeline_layout_hash] = pipeline_layout;
        pipeline_layout->build();

        return pipeline_layout;
    }

    auto DeviceVulkan::merge_descriptor_set_layouts(const DescriptorSetLayout& layout_a, const DescriptorSetLayout& layout_b)
        -> Shared<DescriptorSetLayout>
    {
        std::unordered_map<u32, vk::DescriptorSetLayoutBinding> binding_map{};

        const auto& bindings_a = layout_a.get_bindings();
        for (const auto& binding : bindings_a)
        {
            binding_map[binding.binding] = binding;
        }

        const auto& bindings_b = layout_b.get_bindings();
        for (const auto& binding : bindings_b)
        {
            if (!binding_map.contains(binding.binding))
            {
                binding_map[binding.binding] = binding;
                continue;
            }

            // Merge bindings
            auto& existing_binding = binding_map[binding.binding];
            ASSERT(binding.descriptorType == existing_binding.descriptorType);

            existing_binding.descriptorCount = std::max(existing_binding.descriptorCount, binding.descriptorCount);
            existing_binding.stageFlags |= binding.stageFlags;
        }

        auto merged_layout = CreateShared<DescriptorSetLayout>(m_device.get());

        for (auto& [binding_loc, binding] : binding_map)
        {
            merged_layout->add_binding(binding.binding, binding.descriptorType, binding.stageFlags);
        }

        return merged_layout;
    }

    auto DeviceVulkan::merge_pipeline_layouts(const PipelineLayout& layout_a, const PipelineLayout& layout_b) -> Shared<PipelineLayout>
    {
        auto merged_layout = CreateShared<PipelineLayout>(m_device.get());

        if (layout_a.has_push_constant_range() || layout_b.has_push_constant_range())
        {
            if (layout_a.has_push_constant_range() && layout_b.has_push_constant_range())
            {
                vk::PushConstantRange push_block{};

                const auto& push_block_a = layout_a.get_push_constant_range(0);
                const auto& push_block_b = layout_b.get_push_constant_range(0);

                push_block.size = std::max(push_block_a.size, push_block_b.size);
                push_block.stageFlags = push_block_a.stageFlags | push_block_b.stageFlags;

                merged_layout->add_push_constant_buffer(push_block.offset, push_block.size, push_block.stageFlags);
            }
            else if (layout_a.has_push_constant_range())
            {
                const auto& push_block = layout_a.get_push_constant_range(0);
                merged_layout->add_push_constant_buffer(push_block.offset, push_block.size, push_block.stageFlags);
            }
            else if (layout_b.has_push_constant_range())
            {
                const auto& push_block = layout_b.get_push_constant_range(0);
                merged_layout->add_push_constant_buffer(push_block.offset, push_block.size, push_block.stageFlags);
            }
        }

        std::vector<Shared<DescriptorSetLayout>> merged_set_layouts{};
        for (const auto& set_layout : layout_a.get_set_layouts())
        {
            merged_set_layouts.push_back(set_layout);
        }

        const auto& set_layouts_b = layout_b.get_set_layouts();
        for (u32 set = 0; set < set_layouts_b.size(); ++set)
        {
            const auto& set_layout = set_layouts_b.at(set);

            if (merged_set_layouts.size() < CAST_U32(set + 1))
            {
                merged_set_layouts.resize(CAST_U32(set + 1));
                merged_set_layouts[set] = set_layout;
                continue;
            }

            merged_set_layouts[set] = merge_descriptor_set_layouts(*merged_set_layouts[set], *set_layout);
        }

        for (u32 set = 0; set < merged_set_layouts.size(); ++set)
            merged_layout->add_set_layout(set, merged_set_layouts.at(set));

        return merged_layout;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                          VkDebugUtilsMessageTypeFlagsEXT /*message_type*/,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                          void* /*user_data*/)
    {
        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LOG_WARN("Vulkan - MessageId = {} : [{}]\n{}\n",
                     callback_data->messageIdNumber,
                     callback_data->pMessageIdName,
                     callback_data->pMessage);
        }
        else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LOG_ERROR("Vulkan - MessageId = {} : [{}]\n{}\n",
                      callback_data->messageIdNumber,
                      callback_data->pMessageIdName,
                      callback_data->pMessage);
        }
        return VK_FALSE;
    }
}
