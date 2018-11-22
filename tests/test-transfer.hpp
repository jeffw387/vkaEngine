#pragma once

#include "CommandPool.hpp"
#include "Fence.hpp"
#include "Device.hpp"

struct Transfer {
  std::unique_ptr<vka::CommandPool> pool;
  std::weak_ptr<vka::CommandBuffer> cmd;
  std::unique_ptr<vka::Fence> fence;

  Transfer() {}
  Transfer(vka::Device* device) :
  pool(device->createCommandPool(true, false)),
  cmd(pool->allocateCommandBuffer()),
  fence(device->createFence(false)) {}
};