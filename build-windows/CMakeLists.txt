cmake_minimum_required(VERSION 3.22)

project(vulkan-minimal-windows LANGUAGES CXX)

## --------------- external libraries folder variables 

set( EXTERNAL_REPOS_ROOT "C:/Users/Carlo/git-ct/otros" )
set( GLFW_ROOT "C:/GLFW3/glfw-3.4.bin.WIN64" )

set( GLFW_INCLUDE_DIR "${GLFW_ROOT}/include/")
set( GLFW_LINK_ITEM "${GLFW_ROOT}/lib-vc2022/glfw3.lib")
set( IMGUI_DIR "${EXTERNAL_REPOS_ROOT}/imgui.git" )
set( STB_DIR "${EXTERNAL_REPOS_ROOT}/stb.git" )

## ---------------------------------

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



# Vulkan SDK
find_package(Vulkan REQUIRED)
cmake_path(GET Vulkan_INCLUDE_DIR PARENT_PATH VULKAN_ROOT)

set( SHADERC_INCLUDE_DIR "${VULKAN_ROOT}")
set( VULKAN_BIN_DIR "${VULKAN_ROOT}/Bin")
file(GLOB VULKAN_BIN_PDB_FILES CONFIGURE_DEPENDS "${VULKAN_BIN_DIR}/*.pdb")

# PDB names usually referenced by objects inside shaderc_combinedd.lib.
set(_SHADERC_DEBUG_PDB_NAMES
    "glslangd.pdb"
    "SPIRV-Toolsd.pdb"
    "SPIRV-Tools-optd.pdb"
)

set(HAS_REQUIRED_SHADERC_PDBS TRUE)
foreach(_shaderc_pdb IN LISTS _SHADERC_DEBUG_PDB_NAMES)
    if(NOT EXISTS "${VULKAN_BIN_DIR}/${_shaderc_pdb}")
        set(HAS_REQUIRED_SHADERC_PDBS FALSE)
    endif()
endforeach()

set(_COPY_VULKAN_PDBS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/copy_vulkan_pdbs.cmake")
file(WRITE "${_COPY_VULKAN_PDBS_SCRIPT}" [=[
    if(NOT DEFINED VCFG OR NOT VCFG STREQUAL "Debug")
        return()
    endif()

    if(NOT DEFINED VULKAN_BIN_DIR OR NOT EXISTS "${VULKAN_BIN_DIR}")
        return()
    endif()

    if(NOT DEFINED DEST_DIR OR DEST_DIR STREQUAL "")
        return()
    endif()

    file(GLOB _vulkan_pdbs "${VULKAN_BIN_DIR}/*.pdb")
    if(_vulkan_pdbs)
        file(COPY ${_vulkan_pdbs} DESTINATION "${DEST_DIR}")
    endif()
]=])


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

# Build shared app sources once and reuse in both executables.
add_library(app-common STATIC ${COMMON_APP_SOURCES})
target_include_directories(app-common PUBLIC
    "${SRC_ROOT}/vkhc"
    "${SRC_ROOT}/ilc"
    "${STB_DIR}"
    "${SHADERC_INCLUDE_DIR}"
    "${GLFW_INCLUDE_DIR}"
    "${Vulkan_INCLUDE_DIR}"
    "${IMGUI_DIR}"
)
target_link_libraries(app-common PUBLIC
    imgui
    ${GLFW_LINK_ITEM}
    Vulkan::Vulkan
    debug "${VULKAN_ROOT}/Lib/shaderc_combinedd.lib"
    optimized "${VULKAN_ROOT}/Lib/shaderc_combined.lib"
)

function(configure_debug_linker_warnings TARGET_NAME)
    # Make SDK PDBs available in the target output folder before link.
    if(VULKAN_BIN_PDB_FILES)
        add_custom_command(TARGET ${TARGET_NAME} PRE_LINK
            COMMAND ${CMAKE_COMMAND}
                -DVCFG=$<CONFIG>
                -DVULKAN_BIN_DIR=${VULKAN_BIN_DIR}
                -DDEST_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>
                -P "${_COPY_VULKAN_PDBS_SCRIPT}"
            VERBATIM
        )
    endif()

    # If required shaderc/SPIRV/glslang debug PDB names are absent in SDK Bin,
    # suppress LNK4099 to keep Debug link output clean.
    if(NOT HAS_REQUIRED_SHADERC_PDBS)
        target_link_options(${TARGET_NAME} PRIVATE
            $<$<CONFIG:Debug>:/IGNORE:4099>
        )
    endif()

    # Avoid Debug/Release CRT conflicts coming from prebuilt third-party libraries.
    target_link_options(${TARGET_NAME} PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:MSVCRT>
    )
endfunction()

function(add_vulkan_app TARGET_NAME APP_MAIN)
    add_executable(${TARGET_NAME}
        ${APP_MAIN}
    )

    configure_debug_linker_warnings(${TARGET_NAME})

    target_link_libraries( ${TARGET_NAME} PRIVATE
        app-common
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
    VULKAN_ROOT
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
