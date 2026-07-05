# Collect engine source files.
file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/external/implementations.cpp"
)

# Collect engine headers (for IDE support).
file(GLOB_RECURSE ENGINE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"    
)

add_library(engine ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Define project root directory.
target_compile_definitions(engine PUBLIC
	PROJECT_ROOT="${CMAKE_SOURCE_DIR}/"
	ENGINE_ROOT="${CMAKE_SOURCE_DIR}/Engine/"
)

target_include_directories(engine
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(engine
	PUBLIC
		d3d12.lib
		dxgi.lib
		dxguid.lib
		Microsoft::DirectX-Headers
		D3D12MemoryAllocator
	PRIVATE
		stb
		cgltf
)

target_compile_definitions(engine PUBLIC UNICODE _UNICODE)
