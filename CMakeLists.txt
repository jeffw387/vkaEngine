cmake_minimum_required(VERSION 3.9)

set(EngineVersionMajor 0)
set(EngineVersionMinor 0)
set(EngineVersionPatch 1)
project(vkaEngineProject 
  LANGUAGES C CXX 
  VERSION ${EngineVersionMajor}.${EngineVersionMinor}.${EngineVersionPatch})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in" 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp" @ONLY)

set(BUILD_SHARED_LIBS OFF)

set(srcDir "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(includeDir "${CMAKE_CURRENT_SOURCE_DIR}/include")
list(APPEND vkaEngineSources 
  "${srcDir}/Engine.cpp"
  "${srcDir}/Instance.cpp"
  "${srcDir}/Surface.cpp"
  "${srcDir}/RenderGraph.cpp"
  "${srcDir}/Buffer.cpp"
  "${srcDir}/Device.cpp")
  
list(APPEND vkaEngineIncludes
  "${includeDir}/version.hpp"
  "${includeDir}/Engine.hpp"
  "${includeDir}/RenderObject.hpp"
  "${includeDir}/Instance.hpp"
  "${includeDir}/Device.hpp"
  "${includeDir}/Buffer.hpp"
  "${includeDir}/QueueTraits.hpp"
  "${includeDir}/overloaded.hpp"
  "${includeDir}/braintree.hpp"
  "${includeDir}/RenderGraph.hpp"
  "${includeDir}/Surface.hpp")
add_library(vkaEngine ${vkaEngineSources})
target_include_directories(vkaEngine PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(vkaEngine PROPERTIES
  CXX_STANDARD 17)

add_custom_command(
  TARGET vkaEngine
  POST_BUILD
  COMMAND clang-format -i -style=file ${vkaEngineSources} ${vkaEngineIncludes})

add_subdirectory(external/VulkanFunctionLoader)
target_link_libraries(vkaEngine PUBLIC VulkanFunctionLoader)

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

add_subdirectory(external/assimp)
if(NOT TARGET assimp)
  message(FATAL_ERROR "assimp target not found")
endif()
target_link_libraries(vkaEngine PUBLIC assimp)

add_subdirectory(external/glfw)
if(NOT TARGET glfw)
  message(FATAL_ERROR "glfw target not found")
endif()
target_link_libraries(vkaEngine PUBLIC glfw)

add_subdirectory(external/bullet3)
set(RequiredBulletComponents Bullet3Common BulletDynamics BulletCollision LinearMath)
foreach(component ${RequiredBulletComponents})
  if(NOT TARGET ${component})
    message(FATAL_ERROR "${component} target from BulletPhysics not found")
  endif()
endforeach()

add_subdirectory(external/tbb)
if(NOT TARGET tbb_static)
  message(FATAL_ERROR "tbb_static target not found")
  endif()
target_link_libraries(vkaEngine PUBLIC tbb_static)

add_subdirectory(tests)