cmake_minimum_required(VERSION 3.20)

if(EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten" OR CMAKE_TOOLCHAIN_FILE MATCHES "[Ee]mscripten")
    set(BUILD_WEBASSEMBLY TRUE)
    set(IMGENIE_BACKEND "OpenGL3")  # Emscripten only supports OpenGL ES
    add_compile_options(-g -fexceptions -fdebug-compilation-dir=${CMAKE_SOURCE_DIR})
    message(STATUS "Build target: WebAssembly (Emscripten)")
else()
    set(BUILD_WEBASSEMBLY FALSE)
    message(STATUS "Build target: Native")
    # Backend selection: OpenGL (default) or Vulkan
    # Emscripten is auto-detected and forces OpenGL ES
    if(NOT DEFINED IMGENIE_BACKEND)
        set(IMGENIE_BACKEND "OpenGL3" CACHE STRING "Graphics backend (OpenGL or Vulkan)")
    endif()
    set_property(CACHE IMGENIE_BACKEND PROPERTY STRINGS "OpenGL3" "Vulkan")
endif()

string(TOLOWER ${IMGENIE_BACKEND} lower_case_imgenie_backend)

set(PROJECT ImEffects_${lower_case_imgenie_backend})

project(${PROJECT} CXX)
enable_language(C CXX)

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_PDB_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}/debug")

set(CMAKE_INSTALL_PREFIX .)
set(CMAKE_INSTALL_BINDIR .)

if(NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX _debug)
endif()

if(BUILD_WEBASSEMBLY)
    set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
endif()

include(cmake/3rdparty.cmake)

set(ORIGINAL_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_definitions(-DMSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
    if(NOT BUILD_WEBASSEMBLY)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    endif()
    if((CMAKE_BUILD_TYPE STREQUAL "Debug") OR
        ($<CONFIG> STREQUAL "Debug"))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
endif()

if(BUILD_WEBASSEMBLY)
    add_definitions(-DEMSCRIPTEN)
    add_definitions(-DIMGUI_IMPL_OPENGL_ES3)
    remove_definitions(-DWIN32)
    remove_definitions(-DMSVC)
    remove_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_definitions(${GLFW_DEFINITIONS})
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)

if(NOT BUILD_WEBASSEMBLY)
    file(GLOB_RECURSE CMAKE_SOURCES ${CMAKE_SOURCE_DIR}/cmake/*.*)
    source_group(cmake FILES ${CMAKE_SOURCES})
endif()

# Backend-specific sources
file(GLOB_RECURSE MAIN_SOURCES 
    src/frame/*.*
    src/helpers/*.*
    src/mains/main_${lower_case_imgenie_backend}.cpp)
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX src FILES ${MAIN_SOURCES})
file(GLOB IMGUIIMPL_SOURCES
    3rdparty/imgui/backends/imgui_impl_glfw.cpp
    3rdparty/imgui/backends/imgui_impl_glfw.h
    3rdparty/imgui/backends/imgui_impl_${lower_case_imgenie_backend}.cpp
    3rdparty/imgui/backends/imgui_impl_${lower_case_imgenie_backend}.h)
source_group(src/ImGuiImpl FILES ${IMGUIIMPL_SOURCES})

include_directories(
    ${GLM_INCLUDE_DIR}
    ${PLOG_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT IMGENIE_BACKEND STREQUAL "Vulkan")
    include_directories(
        ${GLAD_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
    )
endif()

add_subdirectory(ImGuiFileDialog)
target_link_libraries(ImGuiFileDialog PRIVATE ${IMGUI_LIBRARIES})
target_include_directories(ImGuiFileDialog PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(ImGuiFileDialog PROPERTIES FOLDER libs)

add_subdirectory(ImCoolBar)
target_link_libraries(ImCoolBar PRIVATE ${IMGUI_LIBRARIES})
target_include_directories(ImCoolBar PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(ImCoolBar PROPERTIES FOLDER libs)

add_subdirectory(ImGenie)
target_link_libraries(ImGenie PRIVATE ${IMGUI_LIBRARIES})
target_include_directories(ImGenie PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(ImGenie PROPERTIES FOLDER libs)

set(SOURCES
    ${RES_SOURCES}
    ${SRC_SOURCES}
    ${MAIN_SOURCES}
    ${CMAKE_SOURCES}
    ${IMGUIIMPL_SOURCES}
)

add_executable(${PROJECT} ${SOURCES})

if(LINUX)
    target_link_libraries(${PROJECT} PRIVATE X11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

if(UNIX)
    target_compile_options(${PROJECT} PUBLIC -Wno-unknown-pragmas)
endif()

if(BUILD_WEBASSEMBLY)
    set_target_properties(${PROJECT} PROPERTIES
        OUTPUT_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}_${ARCH}"
        SUFFIX ".html"
    )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${FINAL_BIN_DIR}/res@/res -lidbfs.js -sEXPORTED_RUNTIME_METHODS=['ccall']")
else()
    set_target_properties(${PROJECT} PROPERTIES
        OUTPUT_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}"
    )
endif()

set(CMAKE_CXX_STANDARD 11)
set_target_properties(${PROJECT} PROPERTIES CXX_STANDARD 11)
set_target_properties(ImCoolBar PROPERTIES CXX_STANDARD 11)
set_target_properties(ImGenie PROPERTIES CXX_STANDARD 11)
set_target_properties(imgui PROPERTIES CXX_STANDARD 11)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

if(BUILD_WEBASSEMBLY)
    # Emscripten traite les chaînes commençant par -s comme des options de link
    target_link_libraries(${PROJECT}
        ${IMGUI_LIBRARIES}
        ImGuiFileDialog
		ImCoolBar
        ImGenie
        "-s USE_GLFW=3"
        "-s USE_WEBGL2=1"
        "-s FULL_ES3=1"
        "-s USE_GLFW=3"
        "-s WASM=1"
        "-s FORCE_FILESYSTEM=1"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s INITIAL_MEMORY=256MB"
        "-s ASSERTIONS=1"
        "-s SAFE_HEAP=1" "-s ERROR_ON_UNDEFINED_SYMBOLS=0"
        "-fexceptions"
        "-g"
        "--bind"
        "--source-map-base http://localhost:8000/"
    )
elseif(IMGENIE_BACKEND STREQUAL "Vulkan")
    target_link_libraries(${PROJECT} PRIVATE
        ${GLFW_LIBRARIES}
        ${IMGUI_LIBRARIES}
        Vulkan::Vulkan
        ImGuiFileDialog
		ImCoolBar
        ImGenie
    )
else()
    target_link_libraries(${PROJECT} PRIVATE
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${GLAD_LIBRARIES}
        ${IMGUI_LIBRARIES}
        ImGuiFileDialog
		ImCoolBar
        ImGenie
    )
endif()

set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(${PROJECT} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${FINAL_BIN_DIR}")
endif()
