cmake_minimum_required(VERSION 3.9)

project(vkaEngineProject LANGUAGES CXX)

add_library(vkaEngine "src/Instance.cpp")
target_include_directories(vkaEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(external/VulkanFunctionLoader)
target_link_libraries(vkaEngine PUBLIC VulkanFunctionLoader)

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

add_subdirectory(external/bullet3)
set(RequiredBulletComponents Bullet3Common BulletDynamics BulletCollision LinearMath)
foreach(component ${RequiredBulletComponents})
  if(NOT TARGET ${component})
    message(FATAL_ERROR "${component} target from BulletPhysics not found")
  endif()
endforeach()