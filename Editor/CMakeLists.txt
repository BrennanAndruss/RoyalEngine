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
