# vulkan-minimal
A minimal, portable Vulkan example that uses GLFW (windowing), STB (texture loading), and IMGUI (UI).
It can be built and run on macOS (with MoltenVK) and Linux.

## Compilation

You can build from the command line using the makefiles in `build-linux` and `build-macos`.

On Windows, build with CMake from `build-windows/CMakeLists.txt`.

### Windows (CMake)

(work in progress)
Current CMake configuration builds two executable targets:

- `app-2d` (entry point in `src/app-2d/main.cpp`)
- `app-2dtess` (entry point in `src/app-2dtess/main.cpp`)

<!-- From repository root, configure and build using:

```powershell
cmake -S build-windows -B build-windows/out `
	-DIMGUI_DIR=C:/path/to/imgui `
	-DSTB_DIR=C:/path/to/stb

cmake --build build-windows/out --config Release
```

Generated executables are written to `build-windows/bin`. -->

## Requirements

### Linux

Install the following packages:

- **GLM**: install package `libglm-dev`.
- **GLFW**: window-management interface. Install package `libglfw3-dev`.
- **Vulkan SDK**: Vulkan libraries and headers. Install package `libvulkan-dev` (it may already be installed with `libglfw3-dev`).
- **Vulkan SDK** (_shader compiler_): install package `libshaderc-dev`.
- **IMGUI**: clone the IMGUI GitHub repository (https://github.com/ocornut/imgui) and set the corresponding variable in the `makefile` to point to that folder.
- **STB**: clone the STB GitHub repository (https://github.com/nothings/stb) and set the corresponding variable in the `makefile`.

If you plan to use Visual Studio Code, you can use the `.code-workspace` file, but you will need to adapt the library include paths.


### MacOS

You will need to install:

- **Xcode**: install with Command Line Tools and the Apple UI/graphics frameworks (_Cocoa_, _Metal_, etc.).
- **MoltenVK** (_Vulkan SDK_): for macOS, download and install from https://vulkan.lunarg.com/sdk/home.
- **IMGUI**: clone the IMGUI GitHub repository (https://github.com/ocornut/imgui) and set the corresponding variable in the `makefile` to point to that folder.
- **STB**: clone the STB GitHub repository (https://github.com/nothings/stb) and set the corresponding variable in the `makefile`.
- **GLFW3**: can be installed with Homebrew (https://formulae.brew.sh/formula/glfw), or installed directly (https://www.glfw.org/download.html).

If you plan to use Visual Studio Code, you can use the `.code-workspace` file, but you will need to adapt the library include paths.

### Windows

Compiling in Windows is not possible yet (this is work in progress). Prerequisites:

- **Visual Studio**: install _Visual Studio 2026_ (version 18), Community edition. The required workload is _Desktop Development with C++_.
- **IMGUI**: clone the IMGUI GitHub repository (https://github.com/ocornut/imgui) and set the corresponding variable in `CMakeLists.txt` to point to that folder. 
- **Vulkan**: download and install the Vulkan SDK from LunarG. By default, it is installed in `C:/VulkanSDK/1.4.350` (May 2026). If you choose another location, update `CMakeLists.txt` accordingly (and the Vulkan include path in the `.workspace` file). This SDK includes GLM.
- **GLFW3**: download and install the Windows precompiled binaries from the official site. Version 3.4 is installed as of May 2026. (I placed it at `C:/GLFW3/glfw-3.4.bin.WIN64`)

If you use the `.code-workspace` file, update the include paths there as well.

<!-- ### Windows

You will need:

- **Vulkan SDK**: install from https://vulkan.lunarg.com/sdk/home (provides Vulkan headers and libraries).
- **GLFW3**: install a package that exports CMake targets (for example through vcpkg), or provide a discoverable GLFW library.
- **GLM**: install headers (for example through vcpkg).
- **shaderc**: install library and headers (for example through vcpkg).
- **IMGUI**: clone https://github.com/ocornut/imgui and pass its path with `-DIMGUI_DIR=...`.
- **STB**: clone https://github.com/nothings/stb and pass its path with `-DSTB_DIR=...`. -->

