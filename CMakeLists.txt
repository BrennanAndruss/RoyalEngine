cmake_minimum_required(VERSION 3.24)
project(RoyalEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT WIN32)
	message(FATAL_ERROR "This project targets Windows/DX12 only.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    directx_headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
    GIT_TAG        v1.614.1
)
FetchContent_MakeAvailable(directx_headers)

FetchContent_Declare(
    d3d12ma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git
    GIT_TAG        v2.0.1
)
FetchContent_MakeAvailable(d3d12ma)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(editor)
