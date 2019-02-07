#include "pipeline.hpp"

#include <catch2/catch.hpp>
#include "platform_glfw.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "device.hpp"
#include "descriptor_set_layout.hpp"
#include "pipeline_layout.hpp"
#include "shader_module.hpp"
#include "render_pass.hpp"
#include "move_into.hpp"

using namespace vka;
