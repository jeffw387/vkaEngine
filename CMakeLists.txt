cmake_minimum_required(VERSION 3.10)

set(EngineVersionMajor 0)
set(EngineVersionMinor 0)
set(EngineVersionPatch 1)
project(vkaEngineProject 
  LANGUAGES C CXX 
  VERSION ${EngineVersionMajor}.${EngineVersionMinor}.${EngineVersionPatch})

set(BUILD_SHARED_LIBS OFF)
  
set(srcDir "${CMAKE_CURRENT_SOURCE_DIR}/src")

configure_file("${srcDir}/version.hpp.in" 
  "${srcDir}/version.hpp" @ONLY)

list(APPEND vkaEngineSources 
  "${srcDir}/Logger.cpp"
  "${srcDir}/sync_helper.cpp"
  "${srcDir}/VmaImplementation.cpp"
  "${srcDir}/GLFW.cpp"
  "${srcDir}/Text.cpp"
  "${srcDir}/Camera.cpp"
  "${srcDir}/Instance.cpp"
  "${srcDir}/Surface.cpp"
  "${srcDir}/Buffer.cpp"
  "${srcDir}/Image.cpp"
  "${srcDir}/Framebuffer.cpp"
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
  "${srcDir}/spookyhash.cpp"
  "${srcDir}/Device.cpp")
  
list(APPEND vkaEngineIncludes
  "${srcDir}/version.hpp"
  "${srcDir}/thsvs_simpler_vulkan_synchronization.h"
  "${srcDir}/Text.hpp"
  "${srcDir}/Logger.hpp"
  "${srcDir}/ObjectPool.hpp"
  "${srcDir}/BasicLoop.hpp"
  "${srcDir}/IO.hpp"
  "${srcDir}/Camera.hpp"
  "${srcDir}/LevelData.hpp"
  "${srcDir}/vulkan_vector.hpp"
  "${srcDir}/RenderPass.hpp"
  "${srcDir}/Instance.hpp"
  "${srcDir}/Device.hpp"
  "${srcDir}/Buffer.hpp"
  "${srcDir}/Image.hpp"
  "${srcDir}/Framebuffer.hpp"
  "${srcDir}/CommandPool.hpp"
  "${srcDir}/DescriptorPool.hpp"
  "${srcDir}/DescriptorSetLayout.hpp"
  "${srcDir}/ShaderModule.hpp"
  "${srcDir}/Swapchain.hpp"
  "${srcDir}/PipelineLayout.hpp"
  "${srcDir}/Pipeline.hpp"
  "${srcDir}/Fence.hpp"
  "${srcDir}/Semaphore.hpp"
  "${srcDir}/VkEnumStrings.hpp"
  "${srcDir}/overloaded.hpp"
  "${srcDir}/Input.hpp"
  "${srcDir}/IntrusiveList.hpp"
  "${srcDir}/QuadTree.hpp"
  "${srcDir}/spookyhash.hpp"
  "${srcDir}/Surface.hpp")
add_library(vkaEngine)
target_sources(vkaEngine PUBLIC ${vkaEngineIncludes} PRIVATE ${vkaEngineSources})
target_include_directories(vkaEngine PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_property(TARGET vkaEngine PROPERTY CXX_STANDARD 17)
set_property(TARGET vkaEngine PROPERTY CXX_STANDARD_REQUIRED ON)
if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set_property(TARGET vkaEngine PROPERTY CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi -Wlifetime")
endif()

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

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_link_libraries(vkaEngine PUBLIC stdc++fs)
endif()

add_subdirectory(external/Catch2)
# target_link_libraries()

add_library(cpp-taskflow INTERFACE)
target_include_directories(cpp-taskflow INTERFACE external/cpp-taskflow)
target_link_libraries(vkaEngine PUBLIC cpp-taskflow)

add_library(box2d)
set(b2d ${CMAKE_CURRENT_SOURCE_DIR}/external/Box2D)
target_sources(box2d PRIVATE 
  ${b2d}/Box2D/Collision/Shapes/b2ChainShape.cpp
  ${b2d}/Box2D/Collision/Shapes/b2CircleShape.cpp
  ${b2d}/Box2D/Collision/Shapes/b2EdgeShape.cpp
  ${b2d}/Box2D/Collision/Shapes/b2PolygonShape.cpp
  ${b2d}/Box2D/Collision/b2BroadPhase.cpp
  ${b2d}/Box2D/Collision/b2CollideCircle.cpp
  ${b2d}/Box2D/Collision/b2CollideEdge.cpp
  ${b2d}/Box2D/Collision/b2CollidePolygon.cpp
  ${b2d}/Box2D/Collision/b2Collision.cpp
  ${b2d}/Box2D/Collision/b2Distance.cpp
  ${b2d}/Box2D/Collision/b2DynamicTree.cpp
  ${b2d}/Box2D/Collision/b2TimeOfImpact.cpp
  ${b2d}/Box2D/Common/b2BlockAllocator.cpp
  ${b2d}/Box2D/Common/b2Draw.cpp
  ${b2d}/Box2D/Common/b2Math.cpp
  ${b2d}/Box2D/Common/b2Settings.cpp
  ${b2d}/Box2D/Common/b2StackAllocator.cpp
  ${b2d}/Box2D/Common/b2Timer.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2CircleContact.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2Contact.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2ContactSolver.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp
  ${b2d}/Box2D/Dynamics/Contacts/b2PolygonContact.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2DistanceJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2FrictionJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2GearJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2Joint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2MotorJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2MouseJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2PulleyJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2RopeJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2WeldJoint.cpp
  ${b2d}/Box2D/Dynamics/Joints/b2WheelJoint.cpp
  ${b2d}/Box2D/Dynamics/b2Body.cpp
  ${b2d}/Box2D/Dynamics/b2ContactManager.cpp
  ${b2d}/Box2D/Dynamics/b2Fixture.cpp
  ${b2d}/Box2D/Dynamics/b2Island.cpp
  ${b2d}/Box2D/Dynamics/b2World.cpp
  ${b2d}/Box2D/Dynamics/b2WorldCallbacks.cpp
  ${b2d}/Box2D/Rope/b2Rope.cpp)
target_include_directories(box2d PUBLIC ${b2d})
target_link_libraries(vkaEngine PUBLIC box2d)

add_subdirectory(external/glslang)
add_dependencies(vkaEngine glslangValidator)
set(SPIRV_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/glslang/StandAlone)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE external/stb)
target_link_libraries(vkaEngine PUBLIC stb)

add_subdirectory(external/msdfgen)
target_link_libraries(vkaEngine PUBLIC lib_msdfgen)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external/outcome")
add_library(outcome INTERFACE)
target_include_directories(outcome INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/external/outcome/single-header")
target_link_libraries(vkaEngine PUBLIC outcome)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external/range-v3")
target_link_libraries(vkaEngine PUBLIC range-v3)

find_package(Vulkan REQUIRED)
if(VULKAN_FOUND)
  message("Vulkan found.")
endif()
message("vk include dirs: ${Vulkan_INCLUDE_DIRS}")
message("vk libs: ${Vulkan_LIBRARIES}")
target_include_directories(vkaEngine PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(vkaEngine PUBLIC ${Vulkan_LIBRARIES})

add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator 
INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/external/VulkanMemoryAllocator/src")
target_link_libraries(vkaEngine PUBLIC VulkanMemoryAllocator)

add_subdirectory(external/glm)
if(NOT TARGET glm)
message(FATAL_ERROR "glm target not found")
endif()
target_link_libraries(vkaEngine PUBLIC glm)

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

function(create_test_target target_name source_file)
  add_executable(${target_name} ${source_file})
  target_link_libraries(${target_name} vkaEngine Catch2)
  set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${target_name} PROPERTY CXX_STANDARD_REQUIRED ON)
  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND clang-format -i -style=file ${source_file})
endfunction()

create_test_target(GLFW.test ${srcDir}/GLFW.test.cpp)
create_test_target(Instance.test ${srcDir}/Instance.test.cpp)
create_test_target(Device.test ${srcDir}/Device.test.cpp)

add_subdirectory(tests)