# vulkan-minimal
A simple portable Vulkan example, uses GLFW (window), STB (loading textures), IMGUI (graphics interface).
Can be compiled and run in MacOS (with MoltenVK) and Linux.

## Compilation 

Can be compiled from the command line using `makefile` files in `build-linux` and `build-macos` (windows pending)

## Requeriments

### Linux 

(this information about Linux requeriments is still incomplete)

Install the following packages:

- `libglfw3-dev`  
- `libvulkan-dev`
- `libshaderc-dev`

Clone github repositories:

- IMGUI: https://github.com/ocornut/imgui
- STB: https://github.com/nothings/stb


### MacOS

You will need to install:

- *XCode* with the command line tools and  all the Apple IU and graphics frameworks (_Cocoa_, _Metal_, etc...)
- *MoltenVK* (_Vulkan SDK_) for MacOs, download and install from: https://vulkan.lunarg.com/sdk/home
- *IMGUI* simply clone the IMGUI Github repository (https://github.com/ocornut/imgui), and set up the corresponding variable in the `makefile` to point to the repository folder
- *STB* clone the STB Github repository (https://github.com/nothings/stb), set the corresponding variable in the `makefile`
- *GLFW3* I installed it as a _homebrew_ package (https://formulae.brew.sh/formula/glfw), but it can also be installed directly (https://www.glfw.org/download.html)

