# Collect editor source files.
file(GLOB_RECURSE EDITOR_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Collect editor headers for IDE support.
file(GLOB_RECURSE EDITOR_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(editor ${EDITOR_SOURCES} ${EDITOR_HEADERS})

target_include_directories(editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(editor PRIVATE engine imgui)

# DXC runtime DLL copy.
add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DXC_DIR}/bin/x64/dxcompiler.dll
        ${DXC_DIR}/bin/x64/dxil.dll
        $<TARGET_FILE_DIR:editor>
)

# Shader compilation.
set(DXC_EXE ${DXC_DIR}/bin/x64/dxc.exe)
set(SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Triangle.hlsl)
set(SHADER_BUILD_DIR ${CMAKE_BINARY_DIR}/CompiledShaders)

add_custom_command(
    OUTPUT ${SHADER_BUILD_DIR}/Triangle_VS.cso ${SHADER_BUILD_DIR}/Triangle_PS.cso
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BUILD_DIR}
    COMMAND ${DXC_EXE} -T vs_6_0 -E VSMain -Fo ${SHADER_BUILD_DIR}/Triangle_VS.cso ${SHADER_SRC}
    COMMAND ${DXC_EXE} -T ps_6_0 -E PSMain -Fo ${SHADER_BUILD_DIR}/Triangle_PS.cso ${SHADER_SRC}
    DEPENDS ${SHADER_SRC}
    COMMENT "Compiling Triangle.hlsl"
)

add_custom_target(CompileShaders ALL
    DEPENDS ${SHADER_BUILD_DIR}/Triangle_VS.cso ${SHADER_BUILD_DIR}/Triangle_PS.cso
)
add_dependencies(editor CompileShaders)

# Copy compiled shaders next to the exe.
add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:editor>/Shaders
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SHADER_BUILD_DIR}/Triangle_VS.cso
        ${SHADER_BUILD_DIR}/Triangle_PS.cso
        $<TARGET_FILE_DIR:editor>/Shaders
)