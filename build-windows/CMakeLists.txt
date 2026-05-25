cmake_minimum_required(VERSION 3.22)

project(vulkan-minimal-windows LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(SRC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../src")
set(APP_2D_MAIN "${SRC_ROOT}/app-2d/main.cpp")
set(APP_2DTESS_MAIN "${SRC_ROOT}/app-2dtess/main.cpp")

file(GLOB VKHC_SOURCES CONFIGURE_DEPENDS "${SRC_ROOT}/vkhc/*.cpp")
file(GLOB ILC_SOURCES CONFIGURE_DEPENDS "${SRC_ROOT}/ilc/*.cpp")

set(COMMON_APP_SOURCES
    ${VKHC_SOURCES}
    ${ILC_SOURCES}
)

# -----------------------------------------------------------------------------
# External dependencies

# Vulkan SDK
find_package(Vulkan REQUIRED)

# GLFW
set(GLFW_LINK_ITEM "")
find_package(glfw3 CONFIG QUIET)
if(TARGET glfw)
    set(GLFW_LINK_ITEM glfw)
elseif(TARGET glfw3)
    set(GLFW_LINK_ITEM glfw3)
else()
    find_library(GLFW_LIBRARY NAMES glfw3 glfw REQUIRED)
    set(GLFW_LINK_ITEM ${GLFW_LIBRARY})
endif()

# GLM headers
find_package(glm CONFIG QUIET)
if(TARGET glm::glm)
    set(HAVE_GLM_TARGET ON)
else()
    find_path(GLM_INCLUDE_DIR glm/glm.hpp REQUIRED)
    set(HAVE_GLM_TARGET OFF)
endif()

# shaderc (used for runtime GLSL -> SPIR-V compilation)
find_path(SHADERC_INCLUDE_DIR shaderc/shaderc.hpp REQUIRED)
find_library(SHADERC_LIBRARY NAMES shaderc_combined shaderc_shared shaderc REQUIRED)

# Dear ImGui and stb are expected as external clone folders (same as existing makefiles)
set(IMGUI_DIR "" CACHE PATH "Path to Dear ImGui repository root")
set(STB_DIR "" CACHE PATH "Path to stb repository root")

if(IMGUI_DIR STREQUAL "")
    message(FATAL_ERROR "Set IMGUI_DIR to your Dear ImGui clone path (contains imgui.h and backends/).")
endif()
if(STB_DIR STREQUAL "")
    message(FATAL_ERROR "Set STB_DIR to your stb clone path (contains stb_image.h).")
endif()

set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

foreach(imgui_src IN LISTS IMGUI_SOURCES)
    if(NOT EXISTS "${imgui_src}")
        message(FATAL_ERROR "Missing ImGui source: ${imgui_src}")
    endif()
endforeach()

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)
target_link_libraries(imgui PUBLIC
    ${GLFW_LINK_ITEM}
    Vulkan::Vulkan
)

function(add_vulkan_app TARGET_NAME APP_MAIN)
    add_executable(${TARGET_NAME}
        ${APP_MAIN}
        ${COMMON_APP_SOURCES}
    )

    target_include_directories(${TARGET_NAME} PRIVATE
        "${SRC_ROOT}/vkhc"
        "${SRC_ROOT}/ilc"
        "${STB_DIR}"
        "${SHADERC_INCLUDE_DIR}"
    )

    if(HAVE_GLM_TARGET)
        target_link_libraries(${TARGET_NAME} PRIVATE glm::glm)
    else()
        target_include_directories(${TARGET_NAME} PRIVATE "${GLM_INCLUDE_DIR}")
    endif()

    target_link_libraries(${TARGET_NAME} PRIVATE
        imgui
        ${GLFW_LINK_ITEM}
        Vulkan::Vulkan
        ${SHADERC_LIBRARY}
    )

    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin"
    )
endfunction()

add_vulkan_app(app-2d ${APP_2D_MAIN})
add_vulkan_app(app-2dtess ${APP_2DTESS_MAIN})
