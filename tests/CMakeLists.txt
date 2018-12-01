message("FROM TESTS: before adding testmain")
set(BUILD_SHARED_LIBS OFF)
add_executable(testmain "testmain.cpp" "Asset.cpp" "Asset.hpp")
target_link_libraries(testmain PRIVATE vkaEngine)

get_target_property(vkaEngineSources vkaEngine SOURCES)
target_sources(testmain PRIVATE ${vkaEngineSources})
set_target_properties(testmain PROPERTIES
  CXX_STANDARD 17)
  
add_custom_command(
  TARGET testmain
  POST_BUILD
  COMMAND clang-format -i -style=file "${CMAKE_CURRENT_SOURCE_DIR}/testmain.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test-text.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test-debug.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test-transfer.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test-p3d-pipeline.hpp"
  )

macro(process_shader fileNames inputDir outputDir)
  foreach(fileName ${${fileNames}})
    set(inputShader "${inputDir}/${fileName}")
    set(outputShader "${outputDir}/${fileName}.spv")
    add_custom_command(
      OUTPUT ${outputShader}
      VERBATIM
      COMMAND ${SPIRV_DIR}/glslangValidator -V ${inputShader} -o ${outputShader}
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

list(APPEND modelFileNames shapes.gltf shapes.bin terrain.gltf terrain.bin)
copy_content(
  modelFileNames
  "${CMAKE_CURRENT_SOURCE_DIR}/content/models"
  "${CMAKE_CURRENT_BINARY_DIR}/content/models")

list(APPEND fontNiocTresniFiles 
  TracerTong_info.pdf
  NiocTresni.ttf
  NiocTresniBold.ttf
  NiocTresniExtraBold.ttf
  NiocTresniFade.ttf
  NiocTresniSolid.ttf)
copy_content(
  fontNiocTresniFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/content/fonts/NiocTresni"
  "${CMAKE_CURRENT_BINARY_DIR}/content/fonts/NiocTresni"
)

set(aerovias AeroviasBrasilNF.ttf)
copy_content(
  aerovias
  "${CMAKE_CURRENT_SOURCE_DIR}/content/fonts"
  "${CMAKE_CURRENT_BINARY_DIR}/content/fonts"
)

set(liberation LiberationSans-Regular.ttf)
copy_content(
  liberation
  "${CMAKE_CURRENT_SOURCE_DIR}/content/fonts"
  "${CMAKE_CURRENT_BINARY_DIR}/content/fonts"
)

set(anke Anke.ttf)
copy_content(
  anke
  "${CMAKE_CURRENT_SOURCE_DIR}/content/fonts/Anke"
  "${CMAKE_CURRENT_BINARY_DIR}/content/fonts/Anke"
)

list(APPEND shaderFileNames shader.vert shader.frag text.vert text.frag)
process_shader(
  shaderFileNames
  "${CMAKE_CURRENT_SOURCE_DIR}/content/shaders"
  "${CMAKE_CURRENT_BINARY_DIR}/content/shaders")