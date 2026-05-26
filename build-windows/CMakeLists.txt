cmake_minimum_required(VERSION 3.22)

project(vulkan-minimal-windows LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_compile_options(/W4 /permissive- /Zc:__cplusplus)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)


set( SRC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../src")
set( REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set( APP_2D_MAIN "${SRC_ROOT}/app-2d/main.cpp")
set( APP_2DTESS_MAIN "${SRC_ROOT}/app-2dtess/main.cpp")

file( GLOB VKHC_SOURCES CONFIGURE_DEPENDS "${SRC_ROOT}/vkhc/*.cpp")
file( GLOB ILC_SOURCES CONFIGURE_DEPENDS "${SRC_ROOT}/ilc/*.cpp")

set(COMMON_APP_SOURCES
    ${VKHC_SOURCES}
    ${ILC_SOURCES}
)

# -----------------------------------------------------------------------------
# External dependencies

set( EXTERNAL_REPOS_ROOT "C:/Users/Carlo/git-ct/otros" )
set( GLFW_ROOT "C:/GLFW3/glfw-3.4.bin.WIN64" )

set( GLFW_INCLUDE_DIR "${GLFW_ROOT}/include/")
set( GLFW_LINK_ITEM "${GLFW_ROOT}/lib-vc2022/glfw3.lib")
set( IMGUI_DIR "${EXTERNAL_REPOS_ROOT}/imgui.git" )
set( STB_DIR "${EXTERNAL_REPOS_ROOT}/stb.git" )

# Vulkan SDK
find_package(Vulkan REQUIRED)
set( SHADERC_INCLUDE_DIR "${VULKAN_ROOT}")


# Dear ImGui and stb are expected as external clone folders (same as existing makefiles)
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
    "${GLFW_INCLUDE_DIR}"
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
        "${GLFW_INCLUDE_DIR}"
        "${Vulkan_INCLUDE_DIR}"  ## para que encuentre los includes de GLM que están en als carpetas de Vulkan (suponiendo que ha encontrado vulkan)
        "${IMGUI_DIR}"  # so imgui includes are found
    )


    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin"
    )
endfunction()

add_vulkan_app( app-2d ${APP_2D_MAIN} )
add_vulkan_app( app-2dtess ${APP_2DTESS_MAIN} )

# Debug helper: print all CMake variables and their current values.
get_cmake_property(_all_vars VARIABLES)
list(SORT _all_vars)
message(STATUS "----- CMake Variables (begin) -----")
foreach(_var IN LISTS _all_vars)
    if(DEFINED ${_var})
        message(STATUS "${_var} = [${${_var}}]")
    endif()
endforeach()
message(STATUS "----- CMake Variables (end) -----")

# Additional helper: print variables explicitly defined in this file.
set(_file_defined_vars
    CMAKE_CXX_STANDARD
    CMAKE_CXX_STANDARD_REQUIRED
    CMAKE_CXX_EXTENSIONS
    SRC_ROOT
    REPO_ROOT
    APP_2D_MAIN
    APP_2DTESS_MAIN
    VKHC_SOURCES
    ILC_SOURCES
    COMMON_APP_SOURCES
    EXTERNAL_REPOS_ROOT
    GLFW_ROOT
    GLFW_INCLUDE_DIR
    GLFW_LINK_ITEM
    IMGUI_DIR
    SHADERC_INCLUDE_DIR
    STB_DIR
    IMGUI_SOURCES
)

message(STATUS "----- File Variables (begin) -----")
foreach(_file_var IN LISTS _file_defined_vars)
    if(DEFINED ${_file_var})
        message(STATUS "${_file_var} = [${${_file_var}}]")
    else()
        message(STATUS "${_file_var} = [<UNDEFINED>]")
    endif()
endforeach()
message(STATUS "----- File Variables (end) -----")
