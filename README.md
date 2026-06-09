# vulkan-minimal
A minimal, portable Vulkan example that uses GLFW (windowing), STB (texture loading), and IMGUI (UI).
It can be built and run on macOS (with MoltenVK) and Linux.

These sources are used to compile two applications:

- A simple 2D applicatioen depicting a triangle.
- A 2D application which demonstrates geometry and tesselation shaders in Vulkan.



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

Prerequisites:

- **Visual Studio**: install _Visual Studio 2026_ (version 18), Community edition. The required workload is _Desktop Development with C++_.
- **IMGUI**: clone the IMGUI GitHub repository (https://github.com/ocornut/imgui) and set the corresponding variable in `CMakeLists.txt` to point to that folder. 
- **Vulkan**: download and install the Vulkan SDK from LunarG. By default, it is installed in `C:/VulkanSDK/1.4.350` (May 2026). If you choose another location, update `CMakeLists.txt` accordingly. This SDK includes GLM.
- **GLFW3**: download and install the Windows precompiled binaries from the official site. Version 3.4 is installed as of May 2026. (I placed it at `C:/GLFW3/glfw-3.4.bin.WIN64`)
**STB** (clone the repo)

If you use the `.code-workspace` file, update the include paths there as well.

## Compiling

### Linux and macOS

Use any terminal application. Just `cd` to either `build-linux` or `build-macos` according to your OS, and then type `make 2d` (for the simple 2D application) or `make 2dtess` for the 2D tessellation and geometry shaders app. Binaries are output to the `bin` subfolder and objects remain in `objs` subfolder. Use `make clean` to remove compilation files and binaries.


### Window 

Use _Visual Studio 2026 Developer Powershell v18_ to compile and link from the command line. It is a powershell terminal app, but with the env vars necessary to compile from the command line. Just `cd` to the  `cmake` folder and type `cmake ..`, this will create all the necessary compilation files. In order to compile the debug version, in the `cmake` folder, type:

`cmake --build .`

This will create both applications and will output binaries to `bin\Debug` folder. If you want to compile the release version, use:

`cmake --build . --config release`

The binary will be stored in the `bin\Release` folder. You can use two script to ease compilation:

- `clean-build.ps1` removes compilation files (in `cmake`), binaries (in `bin`) and object files (in `objs`)
- `build-all.ps1` removes compilation files and compiles the debug version for both apps.

