message("FROM TESTS: before adding testmain")
add_executable(testmain "testmain.cpp")
target_link_libraries(testmain PRIVATE vkaEngine)

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