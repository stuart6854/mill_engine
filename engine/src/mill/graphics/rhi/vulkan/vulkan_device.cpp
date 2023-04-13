#include "vulkan_device.hpp"

#include "mill/core/base.hpp"
#include "mill/core/debug.hpp"
#include "rhi_resource_vulkan.hpp"
#include "vulkan_device.hpp"
#include "vulkan_screen.hpp"
#include "vulkan_context.hpp"
#include "vulkan_view.hpp"
#include "vulkan_helpers.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

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

    /* Pipeline */

    void DeviceVulkan::compile_pipeline_vertex_input_state(const PipelineVertexInputState& state)
    {
        const auto state_hash = state.get_hash();
        if (is_pipeline_vertex_input_state_compiled(state_hash))
            return;

        vk::VertexInputBindingDescription vertex_binding{};
        vertex_binding.setInputRate(vk::VertexInputRate::eVertex);
        vertex_binding.setBinding(0);

        const auto attributes = to_vulkan(state.attributes, vertex_binding.stride);

        vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
        if (!attributes.empty())
        {
            vertex_input_state.setVertexBindingDescriptions(vertex_binding);
            vertex_input_state.setVertexAttributeDescriptions(attributes);
        }

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{};
        input_assembly_state.setTopology(to_vulkan(state.topology));

        vk::GraphicsPipelineLibraryCreateInfoEXT library_info{};
        library_info.setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eVertexInputInterface);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLibraryKHR | vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT);
        pipeline_info.setPVertexInputState(&vertex_input_state);
        pipeline_info.setPInputAssemblyState(&input_assembly_state);
        pipeline_info.setPNext(&library_info);

        vk::UniquePipeline pipeline = m_device->createGraphicsPipelineUnique({}, pipeline_info).value;
        m_compiledPipelineVertexInputStates[state_hash] = std::move(pipeline);
    }

    bool DeviceVulkan::is_pipeline_vertex_input_state_compiled(hasht state_hash)
    {
        return m_compiledPipelineVertexInputStates.contains(state_hash);
    }

    void DeviceVulkan::compile_pipeline_pre_rasterisation_state(const PipelinePreRasterisationState& state)
    {
        const auto state_hash = state.get_hash();
        if (is_pipeline_pre_rasterisation_state_compiled(state_hash))
            return;

        vk::PipelineLayoutCreateInfo layout_info{};
        layout_info.setFlags(vk::PipelineLayoutCreateFlagBits::eIndependentSetsEXT);
        auto layout = m_device->createPipelineLayout(layout_info);

        vk::ShaderModuleCreateInfo module_info{};
        module_info.setCode(state.vertexSpirv);

        vk::PipelineShaderStageCreateInfo stage_info{};
        stage_info.setPNext(&module_info);
        stage_info.setStage(vk::ShaderStageFlagBits::eVertex);
        stage_info.setPName("main");

        const std::vector<vk::DynamicState> dynamic_states{
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        vk::PipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.setDynamicStates(dynamic_states);

        vk::PipelineViewportStateCreateInfo viewport_state{};
        viewport_state.setViewportCount(1);
        viewport_state.setScissorCount(1);

        vk::PipelineRasterizationStateCreateInfo rasterisation_state{};
        rasterisation_state.setPolygonMode(to_vulkan(state.fillMode));
        rasterisation_state.setCullMode(to_vulkan(state.cullMode));
        rasterisation_state.setFrontFace(to_vulkan(state.frontFace));
        rasterisation_state.setLineWidth(state.lineWidth);

        vk::GraphicsPipelineLibraryCreateInfoEXT library_info{};
        library_info.setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::ePreRasterizationShaders);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLibraryKHR | vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT);
        pipeline_info.setStages(stage_info);
        pipeline_info.setLayout(layout);
        pipeline_info.setPDynamicState(&dynamic_state);
        pipeline_info.setPViewportState(&viewport_state);
        pipeline_info.setPRasterizationState(&rasterisation_state);
        pipeline_info.setPNext(&library_info);

        vk::UniquePipeline pipeline = m_device->createGraphicsPipelineUnique({}, pipeline_info).value;
        m_compiledPipelinePreRasterisationStates[state_hash] = std::move(pipeline);
    }

    bool DeviceVulkan::is_pipeline_pre_rasterisation_state_compiled(hasht state_hash)
    {
        return m_compiledPipelinePreRasterisationStates.contains(state_hash);
    }

    void DeviceVulkan::compile_pipeline_fragment_stage_state(const PipelineFragmentStageState& state)
    {
        const auto state_hash = state.get_hash();
        if (is_pipeline_fragment_stage_state_compiled(state_hash))
            return;

        vk::PipelineLayoutCreateInfo layout_info{};
        layout_info.setFlags(vk::PipelineLayoutCreateFlagBits::eIndependentSetsEXT);
        auto layout = m_device->createPipelineLayout(layout_info);

        vk::ShaderModuleCreateInfo module_info{};
        module_info.setCode(state.fragmentSpirv);

        vk::PipelineShaderStageCreateInfo stage_info{};
        stage_info.setPNext(&module_info);
        stage_info.setStage(vk::ShaderStageFlagBits::eFragment);
        stage_info.setPName("main");

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};
        vk::PipelineMultisampleStateCreateInfo multisample_state{};

        vk::GraphicsPipelineLibraryCreateInfoEXT library_info{};
        library_info.setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentShader);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLibraryKHR | vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT);
        pipeline_info.setStages(stage_info);
        pipeline_info.setLayout(layout);
        pipeline_info.setPDepthStencilState(&depth_stencil_state);
        pipeline_info.setPMultisampleState(&multisample_state);
        pipeline_info.setPNext(&library_info);

        vk::UniquePipeline pipeline = m_device->createGraphicsPipelineUnique({}, pipeline_info).value;
        m_compiledPipelineFragmentStageStates[state_hash] = std::move(pipeline);
    }

    bool DeviceVulkan::is_pipeline_fragment_stage_state_compiled(hasht state_hash)
    {
        return m_compiledPipelineFragmentStageStates.contains(state_hash);
    }

    void DeviceVulkan::compile_pipeline_fragment_output_state(const PipelineFragmentOutputState& state)
    {
        const auto state_hash = state.get_hash();
        if (is_pipeline_fragment_output_state_compiled(state_hash))
            return;

        vk::PipelineColorBlendAttachmentState blend_attachment{};
        blend_attachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        blend_attachment.setBlendEnable(state.enableColorBlend);
        vk::PipelineColorBlendStateCreateInfo color_blend_state{};
        color_blend_state.setAttachments(blend_attachment);

        vk::PipelineMultisampleStateCreateInfo multisample_state{};

        std::vector<vk::Format> colorFormats(state.colorAttachmentFormats.size());
        for (u32 i = 0; i < colorFormats.size(); ++i)
            colorFormats[i] = to_vulkan(state.colorAttachmentFormats[i]);

        vk::PipelineRenderingCreateInfo rendering_info{};
        rendering_info.setColorAttachmentFormats(colorFormats);
        rendering_info.setDepthAttachmentFormat(to_vulkan(state.depthAttachmentFormat));

        vk::GraphicsPipelineLibraryCreateInfoEXT library_info{};
        library_info.setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentOutputInterface);
        library_info.setPNext(&rendering_info);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLibraryKHR | vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT);
        pipeline_info.setPColorBlendState(&color_blend_state);
        pipeline_info.setPMultisampleState(&multisample_state);
        pipeline_info.setPNext(&library_info);

        vk::UniquePipeline pipeline = m_device->createGraphicsPipelineUnique({}, pipeline_info).value;
        m_compiledPipelineFragmentOutputStates[state_hash] = std::move(pipeline);
    }

    bool DeviceVulkan::is_pipeline_fragment_output_state_compiled(hasht state_hash)
    {
        return m_compiledPipelineFragmentOutputStates.contains(state_hash);
    }

    void DeviceVulkan::compile_pipeline(const PipelineState& state)
    {
        hasht compiledPipelineHash{};
        std::vector<vk::Pipeline> libraries{};

        // Vertex Input State
        hash_combine(compiledPipelineHash, state.vertexInputStateHash);
        if (state.vertexInputStateHash != 0)
        {
            libraries.push_back(m_compiledPipelineVertexInputStates.at(state.vertexInputStateHash).get());
        }

        // Pre Rasterisation State
        hash_combine(compiledPipelineHash, state.preRasterisationStateHash);
        if (state.preRasterisationStateHash != 0)
        {
            libraries.push_back(m_compiledPipelinePreRasterisationStates.at(state.preRasterisationStateHash).get());
        }

        // Fragment Stage State
        hash_combine(compiledPipelineHash, state.fragmentStageStateHash);
        if (state.fragmentStageStateHash != 0)
        {
            libraries.push_back(m_compiledPipelineFragmentStageStates.at(state.fragmentStageStateHash).get());
        }

        // Fragment Output State
        hash_combine(compiledPipelineHash, state.fragmentOutputStateHash);
        if (state.fragmentOutputStateHash != 0)
        {
            libraries.push_back(m_compiledPipelineFragmentOutputStates.at(state.fragmentOutputStateHash).get());
        }

        vk::PipelineLibraryCreateInfoKHR linking_info{};
        linking_info.setLibraries(libraries);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setLayout({});
        pipeline_info.setPNext(&linking_info);
        if (g_PipelineLinkTimeOptimisation)
            pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLinkTimeOptimizationEXT);

        vk::UniquePipeline pipeline = m_device->createGraphicsPipelineUnique({}, pipeline_info).value;
        m_compiledPipelines[compiledPipelineHash] = std::move(pipeline);
    }

    bool DeviceVulkan::is_pipeline_compiled(const PipelineState& state)
    {
        const auto pipeline_hash = state.get_hash();
        return m_compiledPipelines.contains(pipeline_hash);
    }

    auto DeviceVulkan::get_pipeline(hasht pipeline_hash) -> vk::Pipeline&
    {
        ASSERT(m_compiledPipelines.contains(pipeline_hash));
        return m_compiledPipelines.at(pipeline_hash).get();
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
