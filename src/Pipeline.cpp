#include "Pipeline.hpp"
#include "VulkanFunctionLoader.hpp"
#include "Device.hpp"

namespace vka {
Pipeline::Pipeline(Device* device) : device(device) {
  
}

void Pipeline::compile() {}
}