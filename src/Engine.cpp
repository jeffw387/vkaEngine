#include "Engine.hpp"
#include <GLFW/glfw3.h>
#include <mutex>
#include <stdexcept>
#include <thread>
#include "Instance.hpp"
#include <memory>
#include <cstdlib>
#include "Device.hpp"
#include "spdlog/spdlog.h"
#include "Surface.hpp"
#include "Config.hpp"

namespace vka {

static void
keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

static void
cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

static void
mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

Engine::Engine(EngineCreateInfo engineCreateInfo)
    : engineCreateInfo(engineCreateInfo),
      updateCallback(engineCreateInfo.updateCallback) {
  assetImportFlags = 0;
  assetImportFlags |= aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                      aiProcess_PreTransformVertices;

  // sometimes valve's overlay causes problems, this next line will disable it
  // on windows
  // _putenv("DISABLE_VK_LAYER_VALVE_steam_overlay_1=1");
  fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogFileName);
  fileSink->set_level(spdlog::level::trace);
  stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  stdoutSink->set_level(spdlog::level::info);
  stderrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  stderrSink->set_level(spdlog::level::info);
  std::vector<spdlog::sink_ptr> sinks = {fileSink, stderrSink};
  multilogger =
      std::make_shared<spdlog::logger>(LoggerName, sinks.begin(), sinks.end());
  spdlog::register_logger(multilogger);
  multilogger->flush_on(spdlog::level::err);
  multilogger->info("Logger initialized.");
  multilogger->info("Creating Engine.");
}

size_t Engine::LoadAsset(const std::string& assetPath) {
  assetBuffer.invalidate();
  auto asset = assetImporter.ReadFile(assetPath, assetImportFlags);
  std::vector<Mesh> assetMeshes;
  std::vector<size_t> assetMaterials;
  for (auto m = 0U; m < asset->mNumMeshes; ++m) {
    assetMeshes.emplace_back();
    auto currentMesh = asset->mMeshes[m];
    for (auto v = 0U; v < currentMesh->mNumVertices; ++v) {
      auto cv = currentMesh->mVertices[v];
      auto cn = currentMesh->mNormals[v];
      assetMeshes.back().vertices.emplace_back(
          Vertex{{cv.x, cv.y, cv.z}, {cn.x, cn.y, cn.z}});
    }
    for (auto f = 0U; f < currentMesh->mNumFaces; ++f) {
      auto& currentFace = currentMesh->mFaces[f];
      for (auto i = 0U; i < currentFace.mNumIndices; ++i) {
        assetMeshes.back().indices.push_back(currentFace.mIndices[i]);
      }
    }
    // TODO: fix material loading and/or linking
    assetMaterials.push_back(0);
  }
  return assetBuffer.addAsset(assetMeshes, assetMaterials);
}

Instance* Engine::createInstance(InstanceCreateInfo instanceCreateInfo) {
  instance = std::make_unique<Instance>(this, instanceCreateInfo);
  return instance.get();
}

void Engine::renderThreadFunc() {
  continueRendering = true;
  while (true) {
    acquireRenderSlot();

    if (!continueRendering || !continueUpdating) {
      return;
    }
  }
}

void Engine::initInputCallbacks() {
  auto surface = instance->getSurface();
  glfwSetMouseButtonCallback(*surface, mouseButtonCallback);
  glfwSetKeyCallback(*surface, keyCallback);
  glfwSetCursorPosCallback(*surface, cursorPositionCallback);
}

void Engine::handleOSMessages() {
  glfwPollEvents();
  if (glfwWindowShouldClose(*instance->getSurface())) {
    continueUpdating = false;
    continueRendering = false;
  }
};

void Engine::run() {
  running = true;
  multilogger->log(spdlog::level::info, "Running engine.");
  if (!instance) {
    multilogger->critical("No instance found!");
  }
  if (!(instance->getSurface())) {
    multilogger->warn("No surface found!");
  }
  if (!(instance->getDevice())) {
    multilogger->critical("No device found!");
  }

  multilogger->info("Creating input callbacks.");
  initInputCallbacks();

  startTime = Clock::now();
  multilogger->info("Starting render thread.");
  std::thread renderThread(&Engine::renderThreadFunc, this);

  continueUpdating = true;
  multilogger->info("Starting update loop.");
  while (true) {
    auto lastUpdateTime = (lastUpdatedIndex != -1)
                              ? indexUpdateTime[lastUpdatedIndex]
                              : startTime;
    auto nextUpdateTime = lastUpdateTime + updateDuration();
    auto currentTime = Clock::now();
    if (nextUpdateTime < currentTime) {
      acquireUpdateSlot();
      if (updateCallback) {
        updateCallback(this);
      }
      setLastUpdated(updateIndex);
    }

    handleOSMessages();

    if (!continueRendering || !continueUpdating) {
      break;
    }
  }
  renderThread.join();
  vkDeviceWaitIdle(*instance->getDevice());
}

void Engine::setLastUpdated(int32_t index) {
  std::scoped_lock updateFinishLock(stateMutex);
  lastUpdatedIndex = index;
}
void Engine::acquireUpdateSlot() {
  std::scoped_lock updateLock(stateMutex);
  for (int32_t i = 0; i < BufferCount; ++i) {
    if (i == renderIndex || i == lastUpdatedIndex) {
      continue;
    }
    updateIndex = i;
    return;
  }
  multilogger->error("Error acquiring update slot, no valid indices.");
}
void Engine::acquireRenderSlot() {
  std::scoped_lock renderLock(stateMutex);
  for (int32_t i = 0; i < BufferCount; ++i) {
    if (i == lastUpdatedIndex) {
      renderIndex = i;
      return;
    }
  }
}
}  // namespace vka