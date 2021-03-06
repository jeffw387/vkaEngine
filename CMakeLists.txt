cmake_minimum_required(VERSION 3.10)

file(READ .version engine_version)
project(vkaEngineProject 
  VERSION ${engine_version})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(bin_dir ${CMAKE_CURRENT_BINARY_DIR})

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.13/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(CONANFILE conanfile.py
                BASIC_SETUP
                PROFILE $ENV{CONANPROFILE}
                BUILD missing)

##
## Modules
##
add_library(vkaEngine)
target_link_libraries(vkaEngine PUBLIC ${CONAN_LIBS})

add_executable(vka_tests ${src_dir}/catch_main.cpp)
target_link_libraries(vka_tests PRIVATE vkaEngine)

function(add_module module_name)
  set(module_source ${src_dir}/${module_name}.cpp)
  set(test_name ${module_name}.test)
  set(test_source ${src_dir}/${test_name}.cpp)
  if(EXISTS ${module_source})
    target_sources(vkaEngine PRIVATE ${module_source})
  endif()
  
  if(EXISTS ${test_source})
    target_sources(vka_tests PRIVATE ${test_source})
  endif()

endfunction()

add_module(variant_helper)
add_module(io)
add_module(logger)
add_module(sync_helper)
add_module(platform_glfw)
add_module(instance)
add_module(physical_device)
add_module(surface)
add_module(input)
add_module(queue_family)
add_module(device)
add_module(memory_allocator)
add_module(queue)
add_module(swapchain)
add_module(descriptor_set_layout)
add_module(descriptor_pool)
add_module(descriptor_set)
add_module(command_pool)
add_module(command_buffer)
add_module(render_pass)
add_module(pipeline_layout)
add_module(shader_module)
add_module(buffer)
add_module(image)
add_module(image_view)
add_module(fence)
add_module(semaphore)
add_module(framebuffer)
add_module(sampler)
add_module(pipeline)