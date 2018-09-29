message("FROM TESTS: before adding testmain")
set(BUILD_SHARED_LIBS OFF)
add_executable(testmain "testmain.cpp")
target_link_libraries(testmain PRIVATE vkaEngine)

get_target_property(vkaEngineSources vkaEngine SOURCES)
target_sources(testmain PRIVATE ${vkaEngineSources})
set_target_properties(testmain PROPERTIES
  CXX_STANDARD 17)
  
add_custom_command(
  TARGET testmain
  POST_BUILD
  COMMAND clang-format -i -style=file "${CMAKE_CURRENT_SOURCE_DIR}/testmain.cpp")

macro(process_shader shaderName inputDir outputDir)
  set(inputShader "${inputDir}/${shaderName}")
  set(outputShader "${outputDir}/${shaderName}")
  add_custom_command(
    OUTPUT ${outputShader}
    COMMAND "glslangvalidator.exe -V ${inputShader} -o ${outputShader}"
    DEPENDS ${inputShader})
    add_custom_target("${shaderName}Target"
    DEPENDS ${outputShader}
    SOURCES ${inputShader})
  add_dependencies(vkaEngine "${shaderName}Target")
  add_custom_command(TARGET "${shaderName}Target"
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${outputDir}")
endmacro()

#process_shader("shader.vert"
#  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/3D"
#  "${CMAKE_CURRENT_BINARY_DIR}/shaders/3D")