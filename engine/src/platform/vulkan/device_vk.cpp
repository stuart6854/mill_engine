#include "device_vk.hpp"

#include "mill/core/base.hpp"
#include "mill/core/debug.hpp"

#include "common_headers_vk.hpp"
#include "context_vk.hpp"
#include "upload_context_vk.hpp"
#include "helpers_vk.hpp"
#include "resources_vk.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mill::platform::vulkan
{
    constexpr auto g_VulkanAPIVersion = VK_API_VERSION_1_3;

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

    DeviceVulkan::DeviceVulkan() = default;

    DeviceVulkan::~DeviceVulkan() = default;

    bool DeviceVulkan::initialise(const AppInfo& app_info)
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

        // Instance
        {
            vk::ApplicationInfo vk_app_info{};
            vk_app_info.setPEngineName("Mill_Engine");
            vk_app_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
            vk_app_info.setPApplicationName(app_info.appName.c_str());
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
                if (!is_instance_ext_supported(ext))
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
                if (!is_instance_layer_supported(layer))
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
            m_instance = vk::createInstance(instance_info);
            if (!m_instance)
            {
                LOG_ERROR("DeviceVulkan - Failed to create instance!");
                return false;
            }

            VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

#if MILL_DEBUG
            m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debug_messenger_info);
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
        }

        // Physical Device
        {
            m_physicalDevice = get_best_dedicated_gpu(m_instance);
            if (!m_physicalDevice)
            {
                LOG_ERROR("DeviceVulkan - Failed to select a physical device!");
                return false;
            }

            auto gpu_props = m_physicalDevice.getProperties();
            LOG_INFO("{}, Driver {}.{}",
                     gpu_props.deviceName,
                     VK_VERSION_MAJOR(gpu_props.driverVersion),
                     VK_VERSION_MINOR(gpu_props.driverVersion));
        }

        // Device
        {
            std::vector<const char*> device_exts{
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            };

            for (const auto& ext : device_exts)
            {
                if (!is_device_ext_supported(m_physicalDevice, ext))
                {
                    LOG_ERROR("DeviceVulkan - The device extension '{}' is not supported!", ext);
                }
            }

            vk::PhysicalDeviceFeatures features{};

            vk::PhysicalDeviceSynchronization2Features sync2_features{};
            sync2_features.setSynchronization2(true);

            vk::PhysicalDeviceDynamicRenderingFeatures dyn_rendering_features{};
            dyn_rendering_features.setDynamicRendering(true);
            dyn_rendering_features.setPNext(&sync2_features);

            m_graphicsQueueFamily = find_graphics_queue_family(m_physicalDevice);
            m_transferQueueFamily = find_transfer_queue_family(m_physicalDevice);

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
            device_info.setPNext(&dyn_rendering_features);

            m_device = m_physicalDevice.createDevice(device_info);
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

            VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device);

            if (m_graphicsQueueFamily != -1)
            {
                m_graphicsQueue = m_device.getQueue(m_graphicsQueueFamily, 0);
                SET_VK_OBJECT_NAME(m_device, VkQueue, m_graphicsQueue, "Main Queue (Graphics)");
            }
            if (m_transferQueueFamily != -1)
            {
                m_transferQueue = m_device.getQueue(m_transferQueueFamily, 0);
                SET_VK_OBJECT_NAME(m_device, VkQueue, m_transferQueue, "Main Queue (Transfer)");
            }
        }

        // Allocator
        {
            vma::AllocatorCreateInfo alloc_info{};
            alloc_info.setInstance(m_instance);
            alloc_info.setPhysicalDevice(m_physicalDevice);
            alloc_info.setDevice(m_device);
            alloc_info.setVulkanApiVersion(g_VulkanAPIVersion);
            m_allocator = vma::createAllocator(alloc_info);
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
            m_descriptorPool = m_device.createDescriptorPool(pool_info);
        }

        m_uploadContext = CreateOwned<UploadContextVulkan>(*this, 81920);

        return true;
    }

    bool DeviceVulkan::shutdown()
    {
        m_uploadContext = nullptr;

        wait_idle();

        for (auto& surface : m_surfaces)
        {
            destroy_swapchain(*surface);
            m_instance.destroy(surface->surface);
        }
        m_surfaces.clear();

        for (u32 i = 0; i < g_FrameBufferCount; ++i)
        {
            // m_device.destroy(m_endOfFrameFences[i]);
        }

        m_device.destroy(m_descriptorPool);
        m_allocator.destroy();
        m_device.destroy();
        m_instance.destroy(m_debugMessenger);
        m_instance.destroy();

        return true;
    }

    void DeviceVulkan::begin_frame()
    {
        m_frameIndex = (m_frameIndex + 1) % g_FrameBufferCount;

        for (auto& surface : m_surfaces)
        {
            surface->imageIndex = m_device.acquireNextImageKHR(surface->swapchain, u64_max, surface->imageReadySemaphore).value;
        }

        // UNUSED(m_device.waitForFences(m_endOfFrameFences[m_frameIndex], true, u64_max));
        // m_device.resetFences(m_endOfFrameFences[m_frameIndex]);
    }

    void DeviceVulkan::end_frame() {}

    void DeviceVulkan::present(Receipt* wait_for)
    {
        if (m_surfaces.empty())
        {
            return;
        }

        std::vector<u32> image_indices(m_surfaces.size());
        std::vector<vk::SwapchainKHR> swapchains(m_surfaces.size());
        for (u32 i = 0; i < m_surfaces.size(); ++i)
        {
            auto& surface = m_surfaces[i];

            image_indices[i] = surface->imageIndex;
            swapchains[i] = surface->swapchain;

            surface->backBufferImages[surface->imageIndex]->layout = vk::ImageLayout::eUndefined;
        }

        vk::PresentInfoKHR present_info{};
        present_info.setImageIndices(image_indices);
        present_info.setSwapchains(swapchains);
        if (wait_for != nullptr)
        {
            present_info.setWaitSemaphores(wait_for->semaphore);
        }

        UNUSED(m_graphicsQueue.presentKHR(present_info));
    }

    void DeviceVulkan::wait_for(Receipt& receipt)
    {
        UNUSED(m_device.waitForFences(receipt.fence, true, u64_max));
        m_device.resetFences(receipt.fence);
    }

    void DeviceVulkan::wait_idle()
    {
        m_graphicsQueue.waitIdle();
    }

    auto DeviceVulkan::submit_context(ContextVulkan& context, Receipt* wait_for) -> Receipt
    {
        auto cmd = context.get_current_cmd();
        auto semaphore = context.get_current_semaphore();
        auto fence = context.get_current_fence();

        std::vector<vk::Semaphore> wait_semaphores{};
        if (wait_for != nullptr)
        {
            wait_semaphores.push_back(wait_for->semaphore);
        }
        for (auto& surface : m_surfaces)
        {
            wait_semaphores.push_back(surface->imageReadySemaphore);
        }

        vk::PipelineStageFlags wait_stage_mask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submit_info{};
        submit_info.setCommandBuffers(cmd);
        submit_info.setWaitDstStageMask(wait_stage_mask);
        submit_info.setWaitSemaphores(wait_semaphores);
        submit_info.setSignalSemaphores(semaphore);

        m_graphicsQueue.submit(submit_info, fence);
        return { semaphore, fence };
    }

    bool DeviceVulkan::add_surface(void* surface_handle, i32 width, i32 height)
    {
        ASSERT(m_device);

        if (get_surface_from_handle(surface_handle) != nullptr)
        {
            LOG_WARN("DeviceVulkan - Cannot create a surface for handle ({}) when it already exists.", fmt::ptr(surface_handle));
            return false;
        }

        auto surface = CreateOwned<Surface>();
        surface->index = m_surfaces.size();
        surface->handle = surface_handle;
        surface->surface = create_surface(surface_handle);
        surface->width = width;
        surface->height = height;
        if (!create_swapchain(*surface, {}))
        {
            LOG_ERROR("DeviceVulkan - Failed to add swapchain for handle ({})!", fmt::ptr(surface_handle));
            return false;
        }
        m_surfaces.push_back(std::move(surface));

        return true;
    }

    void DeviceVulkan::remove_surface(void* surface_handle)
    {
        ASSERT(m_device);

        auto* surface = get_surface_from_handle(surface_handle);
        if (surface == nullptr)
        {
            LOG_WARN("DeviceVulkan - Cannot destroy surface that does not exist for surface ({}).", fmt::ptr(surface_handle));
            return;
        }

        const auto index = surface->index;
        destroy_swapchain(*surface);
        m_instance.destroy(surface->surface);
        m_surfaces.erase(m_surfaces.begin() + index);
    }

    auto DeviceVulkan::create_context() -> Owned<ContextVulkan>
    {
        return CreateOwned<ContextVulkan>(*this);
    }

    auto DeviceVulkan::create_pipeline(const PipelineInit& pipeline_init) -> Owned<PipelineVulkan>
    {
        vk::ShaderModuleCreateInfo vertex_module_info{};
        vertex_module_info.setCode(pipeline_init.vertexSrc);
        auto vertex_module = m_device.createShaderModule(vertex_module_info);

        vk::ShaderModuleCreateInfo fragment_module_info{};
        fragment_module_info.setCode(pipeline_init.fragmentSrc);
        auto fragment_module = m_device.createShaderModule(fragment_module_info);

        std::vector<vk::PipelineShaderStageCreateInfo> stage_infos(2);
        stage_infos[0].setStage(vk::ShaderStageFlagBits::eVertex);
        stage_infos[0].setModule(vertex_module);
        stage_infos[0].setPName("main");

        stage_infos[1].setStage(vk::ShaderStageFlagBits::eFragment);
        stage_infos[1].setModule(fragment_module);
        stage_infos[1].setPName("main");

        std::vector<vk::VertexInputAttributeDescription> attribute_descs(2);
        attribute_descs[0].setBinding(0);
        attribute_descs[0].setLocation(0);
        attribute_descs[0].setFormat(vk::Format::eR32G32B32Sfloat);
        attribute_descs[0].setOffset(0);

        attribute_descs[1].setBinding(0);
        attribute_descs[1].setLocation(1);
        attribute_descs[1].setFormat(vk::Format::eR32G32B32A32Sfloat);
        attribute_descs[1].setOffset(12);

        vk::VertexInputBindingDescription binding_desc{};
        binding_desc.setBinding(0);
        binding_desc.setInputRate(vk::VertexInputRate::eVertex);
        binding_desc.setStride(28);

        vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
        vertex_input_state.setVertexAttributeDescriptions(attribute_descs);
        vertex_input_state.setVertexBindingDescriptions(binding_desc);

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{};
        input_assembly_state.setTopology(pipeline_init.state.topology);
        input_assembly_state.setPrimitiveRestartEnable(false);

        // vk::PipelineTessellationStateCreateInfo tessellation_state{};

        vk::PipelineViewportStateCreateInfo viewport_state{};
        viewport_state.setViewportCount(1);
        viewport_state.setScissorCount(1);

        vk::PipelineRasterizationStateCreateInfo rasterization_state{};
        rasterization_state.setDepthClampEnable(false);
        rasterization_state.setRasterizerDiscardEnable(false);
        rasterization_state.setPolygonMode(pipeline_init.state.polygonMode);
        rasterization_state.setLineWidth(1.0f);
        rasterization_state.setCullMode(pipeline_init.state.cullMode);
        rasterization_state.setFrontFace(pipeline_init.state.frontFace);
        rasterization_state.setDepthBiasEnable(false);

        vk::PipelineMultisampleStateCreateInfo multisample_state{};
        multisample_state.setSampleShadingEnable(false);
        multisample_state.setRasterizationSamples(vk::SampleCountFlagBits::e1);

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};

        std::vector<vk::PipelineColorBlendAttachmentState> color_blends{ 1 };
        color_blends[0].setBlendEnable(false);
        color_blends[0].setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                                          vk::ColorComponentFlagBits::eA);

        vk::PipelineColorBlendStateCreateInfo color_blend_state{};
        color_blend_state.setAttachments(color_blends);

        std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.setDynamicStates(dynamic_states);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setStages(stage_infos);
        pipeline_info.setPVertexInputState(&vertex_input_state);
        pipeline_info.setPInputAssemblyState(&input_assembly_state);
        // pipeline_ci.setPTessellationState(&tessellation_state);
        pipeline_info.setPViewportState(&viewport_state);
        pipeline_info.setPRasterizationState(&rasterization_state);
        pipeline_info.setPMultisampleState(&multisample_state);
        pipeline_info.setPDepthStencilState(&depth_stencil_state);
        pipeline_info.setPColorBlendState(&color_blend_state);
        pipeline_info.setPDynamicState(&dynamic_state);
        pipeline_info.setLayout(pipeline_init.layout->get_layout());

        auto pipeline = m_device.createGraphicsPipeline({}, pipeline_info).value;

        auto pipeline_resource = CreateOwned<PipelineVulkan>();
        pipeline_resource->layout = pipeline_init.layout->get_layout();
        pipeline_resource->pipeline = pipeline;
        pipeline_resource->state = pipeline_init.state;

        m_device.destroy(vertex_module);
        m_device.destroy(fragment_module);

        return pipeline_resource;
    }

    auto DeviceVulkan::create_descriptor_set(DescriptorSetLayout& layout) -> Owned<DescriptorSet>
    {
        return CreateOwned<DescriptorSet>(m_device, m_descriptorPool, layout);
    }

    auto DeviceVulkan::create_buffer(const BufferInit& buffer_init) -> BufferVulkan*
    {
        const auto id = m_nextBufferId++;
        auto [it, result] = m_buffers.try_emplace(id, BufferVulkan());
        ASSERT(result);

        auto& buffer = (*it).second;
        buffer.id = id;
        buffer.size = buffer_init.size;
        buffer.usage = buffer_init.usage;
        buffer.isCPUVisible = buffer_init.isCPUVisible;

        if (!buffer_init.isCPUVisible)
        {
            buffer.usage |= vk::BufferUsageFlagBits::eTransferDst;
        }

        create_buffer(buffer);

        if (buffer_init.isCPUVisible)
        {
            buffer.mappedPtr = m_allocator.mapMemory(buffer.allocation);
        }

        if (buffer_init.initial_data != nullptr)
        {
            if (buffer.isCPUVisible)
            {
                std::memcpy(buffer.mappedPtr, buffer_init.initial_data, buffer_init.size);
            }
            else
            {
                m_uploadContext->add_buffer_upload(buffer, buffer_init.size, buffer_init.initial_data);
                m_uploadContext->flush();
            }
        }

        return &buffer;
    }

    void DeviceVulkan::destroy_context(Owned<ContextVulkan> context)
    {
        context = nullptr;
    }

    void DeviceVulkan::destroy_pipeline(Owned<PipelineVulkan> pipeline)
    {
        m_device.destroy(pipeline->pipeline);
        m_device.destroy(pipeline->layout);
        pipeline = nullptr;
    }

    void DeviceVulkan::destroy_buffer(BufferVulkan* buffer)
    {
        const auto id = buffer->id;
        if (m_buffers.find(id) == m_buffers.end())
        {
            LOG_ERROR("DeviceVulkan - Could not destroy unknown buffer (id = {})!", buffer->id);
            return;
        }

        // #TODO: Add to frame destruction queue
        destroy_buffer(*buffer);

        m_buffers.erase(id);
    }

    auto DeviceVulkan::get_device() const -> vk::Device
    {
        return m_device;
    }

    auto DeviceVulkan::get_graphics_queue_family() const -> i32
    {
        return m_graphicsQueueFamily;
    }

    auto DeviceVulkan::get_graphics_queue() const -> vk::Queue
    {
        return m_graphicsQueue;
    }

    auto DeviceVulkan::get_transfer_queue_family() const -> i32
    {
        return m_transferQueueFamily;
    }

    auto DeviceVulkan::get_transfer_queue() const -> vk::Queue
    {
        return m_transferQueue;
    }

    auto DeviceVulkan::get_upload_context() const -> UploadContextVulkan&
    {
        return *m_uploadContext;
    }

    auto DeviceVulkan::get_current_back_buffer(void* surface_handle) -> ImageVulkan*
    {
        auto* surface = get_surface_from_handle(surface_handle);
        if (surface == nullptr)
        {
            LOG_ERROR("DeviceVulkan - Could not get current back buffer for unknown surface ({})!", fmt::ptr(surface_handle));
            return nullptr;
        }

        return surface->backBufferImages[surface->imageIndex].get();
    }

    bool DeviceVulkan::setup_per_frame_resource()
    {
        for (u32 i = 0; i < g_FrameBufferCount; ++i)
        {
            // m_endOfFrameFences[i] = m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        }
        return true;
    }

    auto DeviceVulkan::get_surface_from_handle(void* handle) -> Surface*
    {
        for (auto& surface : m_surfaces)
        {
            if (surface->handle == handle)
            {
                return surface.get();
            }
        }

        return nullptr;
    }

    auto DeviceVulkan::create_surface(void* surface_handle) -> vk::SurfaceKHR
    {
        vk::SurfaceKHR surface{};
#if MILL_WINDOWS
        vk::Win32SurfaceCreateInfoKHR surface_info{};
        surface_info.setHinstance(GetModuleHandle(nullptr));
        surface_info.setHwnd(static_cast<HWND>(surface_handle));
        surface = m_instance.createWin32SurfaceKHR(surface_info);
#endif

        if (!surface)
        {
            LOG_ERROR("DeviceVulkan - Failed to create SurfaceKHR for surface ({})!", fmt::ptr(surface_handle));
        }
        return surface;
    }

    bool DeviceVulkan::create_swapchain(Surface& surface, vk::SwapchainKHR old_swapchain)
    {
        vk::PresentModeKHR present_mode = select_swapchain_present_mode(m_physicalDevice, surface.surface, true);
        vk::Extent2D extent = select_swapchain_extent(m_physicalDevice, surface.surface, surface.width, surface.height);
        vk::SurfaceFormatKHR surface_format = select_swapchain_surface_format(m_physicalDevice, surface.surface);
        u32 image_count = select_swapchain_image_count(m_physicalDevice, surface.surface);

        vk::SwapchainCreateInfoKHR swapchain_info{};
        swapchain_info.setSurface(surface.surface);
        swapchain_info.setPresentMode(present_mode);
        swapchain_info.setImageExtent(extent);
        swapchain_info.setImageFormat(surface_format.format);
        swapchain_info.setImageColorSpace(surface_format.colorSpace);
        swapchain_info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
        swapchain_info.setImageArrayLayers(1);
        swapchain_info.setMinImageCount(image_count);
        swapchain_info.setOldSwapchain(old_swapchain);

        surface.swapchain = m_device.createSwapchainKHR(swapchain_info);
        if (!surface.swapchain)
        {
            LOG_ERROR("DeviceVulkan - Failed to create swapchain!");
            return false;
        }

        auto view_info = get_image_view_create_info_2d({}, surface_format.format);

        auto images = m_device.getSwapchainImagesKHR(surface.swapchain);
        surface.backBufferImages.clear();
        u32 i = 0;
        for (auto image : images)
        {
            auto image_resource = CreateOwned<ImageVulkan>();
            image_resource->image = image;

            view_info.setImage(image);
            image_resource->view = m_device.createImageView(view_info);
            SET_VK_OBJECT_NAME_INDEXED(m_device, VkImageView, image_resource->view, "Swapchain Image View", i);

            image_resource->extent = extent;
            image_resource->layout = vk::ImageLayout::eUndefined;
            image_resource->range = get_image_subresource_range_2d();

            surface.backBufferImages.push_back(std::move(image_resource));
            ++i;
        }

        surface.imageReadySemaphore = m_device.createSemaphore({});

        return true;
    }

    void DeviceVulkan::destroy_swapchain(Surface& surface)
    {
        m_device.destroy(surface.imageReadySemaphore);

        for (auto& back_buffer_image : surface.backBufferImages)
        {
            m_device.destroy(back_buffer_image->view);
        }

        m_device.destroy(surface.swapchain);
    }

    void DeviceVulkan::create_buffer(BufferVulkan& buffer)
    {
        vk::BufferCreateInfo buffer_info{};
        buffer_info.setSize(buffer.size);
        buffer_info.setUsage(buffer.usage);

        vma::AllocationCreateInfo alloc_info{};
        alloc_info.setUsage(vma::MemoryUsage::eAuto);
        if (buffer.isCPUVisible)
        {
            alloc_info.setFlags(vma::AllocationCreateFlagBits::eHostAccessSequentialWrite);
        }

        auto buffer_allocation = m_allocator.createBuffer(buffer_info, alloc_info);
        if (!buffer_allocation.first || !buffer_allocation.second)
        {
            LOG_ERROR("DeviceVulkan - Failed to create/allocate buffer!");
            return;
        }

        buffer.buffer = buffer_allocation.first;
        buffer.allocation = buffer_allocation.second;

        LOG_DEBUG("DeviceVulkan - Buffer allocated. Size = {}", buffer.size);
    }

    void DeviceVulkan::destroy_buffer(BufferVulkan& buffer)
    {
        if (buffer.mappedPtr != nullptr)
        {
            m_allocator.unmapMemory(buffer.allocation);
        }

        m_allocator.destroyBuffer(buffer.buffer, buffer.allocation);
        buffer.buffer = nullptr;
        buffer.allocation = nullptr;

        LOG_DEBUG("DeviceVulkan - Buffer deallocated. Size = {}", buffer.size);
    }

}
