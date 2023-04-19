#pragma once

#include "core/base.hpp"
#include "core/debug.hpp"
#include "core/engine.hpp"
#include "core/application.hpp"

#include "events/events.hpp"

#include "graphics/static_vertex.hpp"
#include "graphics/rhi.hpp"
#include "platform/platform_interface.hpp"

#include "graphics/static_mesh.hpp"
#include "graphics/scene_renderer.hpp"

#include "input/input_codes.hpp"
#include "input/input.hpp"

#include "resources/resource.hpp"
#include "resources/resource_manager.hpp"

#include "scene/components/transform_component.hpp"
#include "scene/components/static_mesh_component.hpp"
#include "scene/entity.hpp"
#include "scene/scene.hpp"
#include "scene/scene_manager.hpp"

#include "io/data_writer.hpp"
#include "io/data_reader.hpp"
#include "io/binary_writer.hpp"
#include "io/binary_reader.hpp"
#include "utility/signal.hpp"
#include "utility/flags.hpp"
#include "utility/ref_count.hpp"