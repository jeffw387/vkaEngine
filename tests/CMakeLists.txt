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

macro(process_shader fileNames inputDir outputDir)
  foreach(fileName ${${fileNames}})
    set(inputShader "${inputDir}/${fileName}")
    set(outputShader "${outputDir}/${fileName}")
    add_custom_command(
      OUTPUT ${outputShader}
      VERBATIM
      COMMAND glslangValidator -V ${inputShader} -o ${outputShader}
      DEPENDS ${inputShader})
    add_custom_target("${fileName}"
      DEPENDS ${outputShader}
      SOURCES ${inputShader})
    add_dependencies(testmain "${fileName}")
    add_custom_command(TARGET "${fileName}"
      PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${outputDir}")
  endforeach()
endmacro()

macro(copy_content fileNames inputDir outputDir)
  foreach(fileName ${${fileNames}})
    set(inputFile "${inputDir}/${fileName}")
    set(outputFile "${outputDir}/${fileName}")
    add_custom_command(
      TARGET testmain
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${outputDir})
    add_custom_command(
      OUTPUT ${outputFile}
      DEPENDS ${inputFile}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${inputFile} ${outputFile})
    add_custom_target(
      "${fileName}"
      DEPENDS ${outputFile}
      SOURCES ${inputFile}) 
    add_dependencies(testmain "${fileName}")
  endforeach()
endmacro()

list(APPEND modelFileNames shapes.gltf shapes.bin)
copy_content(
  modelFileNames
  "${CMAKE_CURRENT_SOURCE_DIR}/content/models"
  "${CMAKE_CURRENT_BINARY_DIR}/content/models")

list(APPEND shaderFileNames shader.vert shader.frag)
process_shader(
  shaderFileNames
  "${CMAKE_CURRENT_SOURCE_DIR}/content/shaders"
  "${CMAKE_CURRENT_BINARY_DIR}/content/shaders")