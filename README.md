# vulkan-minimal
A simple portable Vulkan example, uses GLFW (window), STB (loading textures), IMGUI (graphics interface).
Can be compiled and run in MacOS (with MoltenVK) and Linux.

## Compilation 

Can be compiled from the command line using `makefile` files in `build-linux` and `build-macos` (Windows pending)

## Requeriments

### Linux 

Install the following packages:

- **GLFW**: install package `libglfw3-dev`  
- **Vulkan SDK**: install package `libvulkan-dev`
- **Vulkan SDK** (_shader compiler_): install package `libshaderc-dev`
- **IMGUI**: simply clone the IMGUI Github repository (https://github.com/ocornut/imgui), and set up the corresponding variable in the `makefile` to point to the repository folder
- **STB**: clone the STB Github repository (https://github.com/nothings/stb), set the corresponding variable in the `makefile`


### MacOS

You will need to install:

- **XCode**: install with the command line tools and  all the Apple IU and graphics frameworks (_Cocoa_, _Metal_, etc...)
- **MoltenVK** (_Vulkan SDK_): for MacOs, download and install from: https://vulkan.lunarg.com/sdk/home
- **IMGUI**: simply clone the IMGUI Github repository (https://github.com/ocornut/imgui), and set up the corresponding variable in the `makefile` to point to the repository folder
- **STB**: clone the STB Github repository (https://github.com/nothings/stb), set the corresponding variable in the `makefile`
- **GLFW3**: I installed it as a _homebrew_ package (https://formulae.brew.sh/formula/glfw), but it can also be installed directly (https://www.glfw.org/download.html)

