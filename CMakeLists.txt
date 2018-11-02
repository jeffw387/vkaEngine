cmake_minimum_required(VERSION 3.9)

set(EngineVersionMajor 0)
set(EngineVersionMinor 0)
set(EngineVersionPatch 1)
project(vkaEngineProject 
  LANGUAGES C CXX 
  VERSION ${EngineVersionMajor}.${EngineVersionMinor}.${EngineVersionPatch})

set(BUILD_SHARED_LIBS OFF)
  
set(srcDir "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(includeDir "${CMAKE_CURRENT_SOURCE_DIR}/include")

configure_file("${includeDir}/version.hpp.in" 
  "${includeDir}/version.hpp" @ONLY)

set(DebugBool "$<$<CONFIG:Debug>:true>")

configure_file("${includeDir}/Config.hpp.in" 
    "${includeDir}/Config.hpp.cfg" @ONLY)

file(GENERATE OUTPUT "${includeDir}/Config.hpp"
  INPUT "${includeDir}/Config.hpp.cfg")

list(APPEND vkaEngineSources 
  "${srcDir}/Logger.cpp"
  "${srcDir}/Text.cpp"
  "${srcDir}/Engine.cpp"
  "${srcDir}/Camera.cpp"
  "${srcDir}/Instance.cpp"
  "${srcDir}/Surface.cpp"
  "${srcDir}/Buffer.cpp"
  "${srcDir}/Image.cpp"
  "${srcDir}/RenderPass.cpp"
  "${srcDir}/CommandPool.cpp"
  "${srcDir}/DescriptorPool.cpp"
  "${srcDir}/DescriptorSetLayout.cpp"
  "${srcDir}/Swapchain.cpp"
  "${srcDir}/ShaderModule.cpp"
  "${srcDir}/PipelineLayout.cpp"
  "${srcDir}/Pipeline.cpp"
  "${srcDir}/Fence.cpp"
  "${srcDir}/Semaphore.cpp"
  "${srcDir}/Device.cpp")
  
list(APPEND vkaEngineIncludes
  "${includeDir}/version.hpp"
  "${includeDir}/Config.hpp"
  "${includeDir}/Text.hpp"
  "${includeDir}/Logger.hpp"
  "${includeDir}/Camera.hpp"
  "${includeDir}/Engine.hpp"
  "${includeDir}/LevelData.hpp"
  "${includeDir}/vulkan_vector.hpp"
  "${includeDir}/RenderPass.hpp"
  "${includeDir}/Instance.hpp"
  "${includeDir}/Device.hpp"
  "${includeDir}/Buffer.hpp"
  "${includeDir}/Image.hpp"
  "${includeDir}/CommandPool.hpp"
  "${includeDir}/DescriptorPool.hpp"
  "${includeDir}/DescriptorSetLayout.hpp"
  "${includeDir}/ShaderModule.hpp"
  "${includeDir}/Swapchain.hpp"
  "${includeDir}/PipelineLayout.hpp"
  "${includeDir}/Pipeline.hpp"
  "${includeDir}/Fence.hpp"
  "${includeDir}/Semaphore.hpp"
  "${includeDir}/VkEnumStrings.hpp"
  "${includeDir}/overloaded.hpp"
  "${includeDir}/Surface.hpp")
add_library(vkaEngine)
target_sources(vkaEngine PUBLIC ${vkaEngineIncludes} PRIVATE ${vkaEngineSources})
target_include_directories(vkaEngine PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(vkaEngine PROPERTIES
  CXX_STANDARD 17)

target_compile_definitions(vkaEngine 
  PUBLIC 
  NOMINMAX
  GLFW_INCLUDE_NONE
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
  _CRT_SECURE_NO_WARNINGS)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "force static build" FORCE)

add_custom_command(
  TARGET vkaEngine
  POST_BUILD
  COMMAND clang-format -i -style=file ${vkaEngineSources} ${vkaEngineIncludes})

#target_link_libraries(vkaEngine PUBLIC stdc++fs)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external/freetype2")
target_link_libraries(vkaEngine PUBLIC freetype)
get_target_property(freetypeIncludes freetype INTERFACE_INCLUDE_DIRECTORIES)
message("Freetype interface includes: ${freetypeIncludes}")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external/outcome")
add_library(outcome INTERFACE)
target_include_directories(outcome INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/external/outcome/single-header")
target_link_libraries(vkaEngine PUBLIC outcome)

find_package(Vulkan REQUIRED)
if(VULKAN_FOUND)
  message("Vulkan found.")
endif()
message("vk include dirs: ${Vulkan_INCLUDE_DIRS}")
message("vk libs: ${Vulkan_LIBRARIES}")
target_include_directories(vkaEngine PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(vkaEngine PUBLIC ${Vulkan_LIBRARIES})

add_library(dear_imgui 
external/imgui/imgui.cpp
external/imgui/imgui_draw.cpp
external/imgui/imgui_widgets.cpp)
target_include_directories(dear_imgui PUBLIC external/imgui)
target_link_libraries(vkaEngine PUBLIC dear_imgui)

add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator 
INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/external/VulkanMemoryAllocator/src")
target_link_libraries(vkaEngine PUBLIC VulkanMemoryAllocator)

add_subdirectory(external/glm)
if(NOT TARGET glm)
message(FATAL_ERROR "glm target not found")
endif()
target_link_libraries(vkaEngine PUBLIC glm)

add_subdirectory(external/gli)
if(NOT TARGET gli)
message(FATAL_ERROR "gli target not found")
endif()
target_link_libraries(vkaEngine PUBLIC gli)

add_library(json_nlohmann INTERFACE)
target_include_directories(
  json_nlohmann
  INTERFACE
  external/json/single_include)
  target_link_libraries(vkaEngine PUBLIC json_nlohmann)
  
  add_subdirectory(external/glfw)
  if(NOT TARGET glfw)
  message(FATAL_ERROR "glfw target not found")
  endif()
  target_link_libraries(vkaEngine PUBLIC glfw)
  
  add_subdirectory(external/spdlog)
  add_library(spdlog::spdlog ALIAS spdlog)
  target_link_libraries(vkaEngine PUBLIC spdlog::spdlog)
  
  add_library(tinygltf INTERFACE)
  target_include_directories(tinygltf INTERFACE external/tinygltf)
  target_link_libraries(tinygltf INTERFACE json_nlohmann)
  target_link_libraries(vkaEngine PUBLIC tinygltf)
  
  add_subdirectory(tests)