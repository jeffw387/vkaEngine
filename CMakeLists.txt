cmake_minimum_required(VERSION 3.10)

set(EngineVersionMajor 0)
set(EngineVersionMinor 0)
set(EngineVersionPatch 1)
project(vkaEngineProject 
  LANGUAGES C CXX 
  VERSION ${EngineVersionMajor}.${EngineVersionMinor}.${EngineVersionPatch})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "force static build" FORCE)
  
set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")
configure_file("${src_dir}/version.hpp.in" 
  "${src_dir}/version.hpp" @ONLY)

##
## Dependencies
##
find_package(Threads)

add_library(cpp-taskflow INTERFACE)
target_include_directories(cpp-taskflow INTERFACE external/cpp-taskflow)

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

add_subdirectory(external/glslang)
set(SPIRV_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/glslang/StandAlone)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE external/stb)

add_subdirectory(external/msdfgen)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external/outcome")
add_library(outcome INTERFACE)
target_include_directories(outcome INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/external/outcome/single-header")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external/range-v3")

find_package(Vulkan REQUIRED)

add_library(vulkan INTERFACE)
target_include_directories(vulkan INTERFACE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(vulkan INTERFACE ${Vulkan_LIBRARIES})

add_library(VulkanMemoryAllocator)
target_include_directories(VulkanMemoryAllocator 
PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/VulkanMemoryAllocator/src")
target_sources(VulkanMemoryAllocator PRIVATE ${src_dir}/VmaImplementation.cpp)
target_link_libraries(VulkanMemoryAllocator PUBLIC vulkan)

add_subdirectory(external/glm)

add_library(json_nlohmann INTERFACE)
target_include_directories(
  json_nlohmann
  INTERFACE
  external/json/single_include)
  
add_subdirectory(external/glfw)
  
add_subdirectory(external/spdlog)
add_library(spdlog::spdlog ALIAS spdlog)

add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE external/tinygltf)
target_link_libraries(tinygltf INTERFACE json_nlohmann)

add_subdirectory(external/Catch2)

##
## End dependencies
##

##
## Modules
##
add_library(vkaEngine INTERFACE)
target_include_directories(vkaEngine INTERFACE ${src_dir})
target_compile_definitions(vkaEngine INTERFACE
  NOMINMAX
  GLFW_INCLUDE_NONE
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
  _CRT_SECURE_NO_WARNINGS)

add_custom_target(tests)

function(setup_target target_name)
  set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${target_name} PROPERTY CXX_STANDARD_REQUIRED ON)
  if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    set_property(TARGET ${target_name} PROPERTY CMAKE_EXE_LINKER_FLAGS 
      "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi -Wlifetime")
  endif()
  if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_link_libraries(${target_name} PUBLIC stdc++fs)
  endif()
endfunction()

function(add_module module_name)
  set(module_source ${src_dir}/${module_name}.cpp)
  set(module_header ${src_dir}/${module_name}.hpp)
  set(test_name ${module_name}.test)
  set(test_source ${src_dir}/${test_name}.cpp)
  if(EXISTS ${module_source})
    add_library(${module_name})
    target_sources(${module_name} PRIVATE ${module_source})
    target_link_libraries(vkaEngine INTERFACE ${module_name})
    add_custom_command(
      TARGET ${module_name}
      POST_BUILD
      COMMAND clang-format -i -style=file ${module_source})
    setup_target(${module_name})
    if(EXISTS ${module_header})
      add_custom_command(
        TARGET ${module_name}
        COMMAND clang-format -i -style=file ${module_header})
    endif()
  else()
    add_library(${module_name} INTERFACE)
  endif()
  
  if(EXISTS ${test_source})
    message("test source: ${test_source}")
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE ${module_name} Catch2)
    add_dependencies(tests ${test_name})
    add_custom_command(
      TARGET ${test_name}
      POST_BUILD
      COMMAND clang-format -i -style=file ${test_source})
    setup_target(${test_name})
  endif()

endfunction()

add_module(Logger)
add_module(sync_helper)
# add_module(VmaImplementation)
add_module(GLFW)
add_module(Text)
add_module(Camera)
add_module(Instance)
add_module(Surface)
add_module(Buffer)
add_module(Image)
add_module(Framebuffer)
add_module(RenderPass)
add_module(CommandPool)
add_module(descriptor_pool)
add_module(Swapchain)
add_module(ShaderModule)
add_module(PipelineLayout)
add_module(Pipeline)
add_module(Fence)
add_module(Semaphore)
add_module(spookyhash)
add_module(Device)
target_link_libraries(sync_helper PUBLIC vulkan)
target_link_libraries(Logger PUBLIC spdlog::spdlog)
target_link_libraries(GLFW PUBLIC vulkan glfw Logger)
target_link_libraries(Instance PUBLIC vulkan Device Surface Logger)
target_link_libraries(Surface PUBLIC vulkan glfw Logger)
target_link_libraries(Device PUBLIC 
  GLFW
  vulkan
  VulkanMemoryAllocator
  outcome
  Logger
  glm
  ShaderModule
  CommandPool
  RenderPass
  PipelineLayout
  Pipeline
  Swapchain
  Fence
  Semaphore
  Framebuffer
  Image
  Buffer)
  target_link_libraries(descriptor_pool INTERFACE GLFW Instance Device outcome)
  target_link_libraries(Buffer PUBLIC vulkan VulkanMemoryAllocator)
  target_link_libraries(Image PUBLIC vulkan VulkanMemoryAllocator)
  target_link_libraries(Swapchain PUBLIC vulkan outcome Logger)
  target_link_libraries(ShaderModule PUBLIC vulkan)
  target_link_libraries(Fence PUBLIC vulkan)
  target_link_libraries(Semaphore PUBLIC vulkan)
  target_link_libraries(Framebuffer PUBLIC vulkan Image range-v3)
  target_link_libraries(RenderPass PUBLIC vulkan Logger)
  target_link_libraries(PipelineLayout PUBLIC vulkan)
  target_link_libraries(Pipeline PUBLIC vulkan Logger)
  target_link_libraries(CommandPool PUBLIC vulkan range-v3 Buffer Image Logger RenderPass Framebuffer Pipeline PipelineLayout Fence sync_helper)

add_dependencies(vkaEngine glslangValidator)

add_subdirectory(tests)