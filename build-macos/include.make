# Included definitions

.SUFFIXES: 
.PHONY: x clean 

compiler:=         g++
cpp_opts:=         -g -Wall
bin_folder:=       ./bin

stb_git:=          ~/git-ct/otros/stb.git## folder with STB sources
vkhc_folder:=      ../src/vkhc## folder with sources for the VKHC classes (Vulkan Helper Classes)
imgui_git:=        ~/git-ct/otros/imgui.git## folder with IMGUI repo clone
vulkan_sdk_base:=  ~/VulkanSDK/1.4.341.1/macOS## vulkan SDK base folder 
homebrew_base:=    /opt/homebrew## homebrew base folder 
ilc_folder:=       ../src/ilc## folder with sources for the ILC classes (Intermediate Level Classes)

## -------------------------------------
## target binary executable  (in)

target:= $(bin_folder)/$(target_base)_exe

## Global, non-configurable options

objs_dir:=         ./objs
compile_options:=  -std=c++20 $(cpp_opts)
link_options:= 

## --------------------------------------------------------------------------------
## Tools options

# App-specific objects
app_sources:=  $(wildcard $(app_src_folder)/*.cpp)
app_objs:=     $(addprefix $(objs_dir)/$(target_base)/, $(notdir $(app_sources:.cpp=.o)))
app_include:=  -I $(app_src_folder)

## VKHC objects 

vkhc_sources:= $(wildcard $(vkhc_folder)/*.cpp)
vkhc_headers:= $(wildcard $(vkhc_folder)/*.h)
vkhc_objs:=    $(addprefix $(objs_dir)/vkhc/, $(notdir $(vkhc_sources:.cpp=.o)))
vkhc_include:= -I $(vkhc_folder)

## ILC objects variables

ilc_sources:= $(wildcard $(ilc_folder)/*.cpp)
ilc_headers:= $(wildcard $(ilc_folder)/*.h)
ilc_objs:=    $(addprefix $(objs_dir)/ilc/, $(notdir $(ilc_sources:.cpp=.o)))
ilc_include:= -I $(ilc_folder)

## IMGUI definitions 
## (IMGUI repository must be cloned in 'imgui_git' path)

imgui_incl:=        -I $(imgui_git)  -I $(imgui_git)/backends
imgui_src_names_1:= imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_tables.cpp 
imgui_src_names_2:= imgui_impl_glfw.cpp imgui_impl_vulkan.cpp
imgui_objs_1:=      $(addprefix $(objs_dir)/imgui/, $(notdir $(imgui_src_names_1:.cpp=.o)))
imgui_objs_2:=      $(addprefix $(objs_dir)/imguibe/, $(notdir $(imgui_src_names_2:.cpp=.o)))
imgui_objs:=        $(imgui_objs_1) $(imgui_objs_2)

## Vulkan SDK (and GLM) definitions

vulkan_include:=    -I $(vulkan_sdk_base)/include
vulkan_libs:=       -lshaderc_util -lshaderc  -lglslang -lSPIRV-tools -lSPIRV  -lslang-compiler -lvulkan 
vulkan_link_flags:= -rpath $(vulkan_sdk_base)/lib -rpath $(vulkan_sdk_base)/Frameworks -F $(vulkan_sdk_base)/Frameworks
vulkan_frameworks:= -framework vulkan

## GLFW definitions 
glfw_libs:= -lglfw3

## Apple specific frameworks
apple_frameworks:= -framework Cocoa -framework QuartzCore -framework Metal -framework MetalKit -framework IOKit -framework CoreFoundation

## Homebrew definitions 
homebrew_include:=    -I $(homebrew_base)/include
homebrew_link_flags:= -L $(homebrew_base)/lib

## STB definitios 
stb_include:= -I $(stb_git)

## ---------------------------------------------------------------------------------
## All libs, objects and include options

libs:= $(glfw_libs) $(vulkan_libs) -ldl -lpthread  $(vulkan_frameworks) $(apple_frameworks)
objs:= $(app_objs) $(vkhc_objs) $(ilc_objs) $(imgui_objs)
incl:= $(app_include) $(vkhc_include) $(ilc_include) $(imgui_incl) $(vulkan_include) $(homebrew_include) $(stb_include)

## Compile and link flags

compile_flags:= $(compile_options) $(incl)
link_flags:=    $(link_options) $(vulkan_link_flags) $(homebrew_link_flags) 

## ---------------------------------------------------------------------------------
## TARGETS 

## main target
x: $(target)
	./$<

## compile an unit from the IMGUI folder onto 'objs/imgui' folder
$(objs_dir)/imgui/%.o: $(imgui_git)/%.cpp makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## compile an unit from the IMGUI backends folder 'objs/imguibe' 
$(objs_dir)/imguibe/%.o: $(imgui_git)/backends/%.cpp makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## compile an unit from the VKHC folder onto 'vkhc/objs'
$(objs_dir)/vkhc/%.o: $(vkhc_folder)/%.cpp $(vkhc_headers) makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## compile an unit from the ILC folder folder onto 'objs/ilc' folder
$(objs_dir)/ilc/%.o: $(ilc_folder)/%.cpp $(ilc_headers) $(vkhc_headers) makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## compile an unit from the APP folder
$(objs_dir)/$(target_base)/%.o: $(app_src_folder)/%.cpp makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## link app
$(target): $(objs) makefile 
	echo "building target: `" $(target) "`"
	$(compiler) $(link_flags) -o $@ $(objs) $(libs) 

## clean all compilation files
clean:
	rm -f $(bin_folder)/*_exe $(objs_dir)/*/*.o

## ---------------------------------------------------------------------------------


