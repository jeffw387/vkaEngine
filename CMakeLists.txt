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
message("Vulkan Libraries: ${Vulkan_LIBRARIES}")

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

add_subdirectory(external/optional)

add_subdirectory(external/expected)

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
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
  _CRT_SECURE_NO_WARNINGS)

add_custom_target(vka_tests)
add_custom_target(vka_tests_tidy)
add_custom_target(vka_tests_format)
add_custom_target(vka_tests_end)

add_dependencies(vka_tests_tidy vka_tests)
add_dependencies(vka_tests_format vka_tests_tidy)
add_dependencies(vka_tests_end vka_tests_format)

set (clang_format_cmd )
add_custom_command(
    TARGET vka_tests_format
    POST_BUILD
    USES_TERMINAL
    VERBATIM
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ./clang-format-all.sh ${src_dir})

add_custom_command(
    TARGET vka_tests_tidy
    POST_BUILD
    COMMAND run-clang-tidy.py ${src_dir} -p ${CMAKE_CURRENT_BINARY_DIR} -fix -header-filter=${src_dir}/*,-gsl_lite.hpp,-spookyhash.*)

function(setup_target target_name)
  set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${target_name} PROPERTY CXX_STANDARD_REQUIRED ON)
  if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    set_property(TARGET ${target_name} PROPERTY CMAKE_EXE_LINKER_FLAGS 
      "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi -Wlifetime")
    set_property(TARGET ${target_name} PROPERTY CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=lld")
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
    setup_target(${module_name})
  else()
    add_library(${module_name} INTERFACE)
  endif()
  
  if(EXISTS ${test_source})
    message("test source: ${test_source}")
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE ${module_name} Catch2)
    add_dependencies(vka_tests ${test_name})
    setup_target(${test_name})
  endif()

endfunction()


add_module(Logger)
target_link_libraries(Logger PUBLIC spdlog::spdlog)

add_module(sync_helper)
target_link_libraries(sync_helper PUBLIC vulkan)

add_module(platform_glfw)
target_link_libraries(platform_glfw PUBLIC vulkan glfw Logger tl_expected)
target_compile_definitions(platform_glfw PUBLIC 
  GLFW_INCLUDE_VULKAN
  GLFW_INCLUDE_NONE)

add_module(instance)
target_link_libraries(instance INTERFACE tl_expected vulkan)
target_link_libraries(instance.test PRIVATE platform_glfw)

add_module(physical_device)
target_link_libraries(physical_device INTERFACE vulkan tl_optional tl_expected)
target_link_libraries(physical_device.test PRIVATE instance platform_glfw)

add_module(queue_family)
target_link_libraries(queue_family INTERFACE vulkan tl_optional)
target_link_libraries(queue_family.test PRIVATE 
  surface 
  instance 
  physical_device 
  platform_glfw)

add_module(device)
target_link_libraries(device INTERFACE tl_expected vulkan)
target_link_libraries(device.test PRIVATE 
  instance 
  queue_family 
  physical_device 
  platform_glfw)

add_module(swapchain)
target_link_libraries(swapchain INTERFACE tl_expected vulkan)
target_link_libraries(swapchain.test PRIVATE 
  instance 
  queue_family 
  physical_device 
  device 
  platform_glfw 
  surface)

# add_module(Text)

# add_module(Camera)

add_module(surface)
target_link_libraries(surface INTERFACE vulkan tl_expected platform_glfw)
target_link_libraries(surface.test PRIVATE instance)

add_module(input)
target_link_libraries(input PUBLIC vulkan surface platform_glfw)

add_module(descriptor_set_layout)
target_link_libraries(descriptor_set_layout INTERFACE vulkan tl_expected)
target_link_libraries(descriptor_set_layout.test PRIVATE 
  platform_glfw 
  physical_device 
  instance 
  device 
  queue_family)

add_module(descriptor_pool)
target_link_libraries(descriptor_pool INTERFACE vulkan tl_expected descriptor_set_layout)
target_link_libraries(descriptor_pool.test PRIVATE 
  platform_glfw 
  physical_device
  instance 
  device 
  queue_family)

add_module(descriptor_set)
target_link_libraries(descriptor_set INTERFACE vulkan tl_expected descriptor_pool descriptor_set_layout)
target_link_libraries(descriptor_set.test PRIVATE 
  platform_glfw 
  physical_device 
  instance 
  device 
  queue_family
  descriptor_pool
  descriptor_set_layout)

add_module(command_pool)
target_link_libraries(command_pool INTERFACE vulkan tl_expected)
target_link_libraries(command_pool.test PRIVATE
  platform_glfw
  physical_device
  instance
  device
  queue_family)

add_module(command_buffer)
target_link_libraries(command_buffer INTERFACE vulkan tl_expected command_pool)
target_link_libraries(command_buffer.test PRIVATE
  platform_glfw
  physical_device
  instance
  device
  queue_family)

add_module(render_pass)
target_link_libraries(render_pass INTERFACE vulkan tl_expected tl_optional)
target_link_libraries(render_pass.test PRIVATE
  platform_glfw
  instance
  physical_device
  queue_family
  device)

# add_module(spookyhash)

add_dependencies(vkaEngine glslangValidator)

# add_subdirectory(tests)