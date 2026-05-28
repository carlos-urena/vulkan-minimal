## Makefile includes for linux (ubuntu) 
##
## Ubuntu packages: libglfw3-dev, libvulkan-dev, libshaderc-dev
## 'app_src_folder' and 'target' must be defined before including this.

.SUFFIXES: 
.PHONY: x clean 

## ------------------------------------------------------------------
## Configurable options



objs_dir:=         ./objs
compiler:=         g++
compile_options:=  -std=c++20 -g -Wall -Wextra -Wno-missing-field-initializers
link_options:= 
bin_folder:=  ./bin

imgui_git   := ~/git-ct/otros/imgui.git## imgui repository clone folder
stb_git     := ~/git-ct/otros/stb.git## folder with STB repository clone
vkhc_folder := ../src/vkhc## folder with sources for the VKHC classes (Vulkan Helper Classes)
ilc_folder  := ../src/ilc## folder with sources for the ILC classes (Intermediate Level Classes)

## -------------------------------------
## target binary executable  (in)

target:= $(bin_folder)/$(target_base)_exe

## --------------------------------------------------------------------------------
## folder for library sources and link files 

stb_include:= -I $(stb_git)

# App-specific variables
app_sources:=  $(wildcard $(app_src_folder)/*.cpp)
app_objs:=     $(addprefix $(objs_dir)/$(target_base)/, $(notdir $(app_sources:.cpp=.o)))
app_include:=  -I $(app_src_folder)

## VKHC objects variables

vkhc_sources:= $(wildcard $(vkhc_folder)/*.cpp)
vkhc_headers:= $(wildcard $(vkhc_folder)/*.h)
vkhc_objs:=    $(addprefix $(objs_dir)/vkhc/, $(notdir $(vkhc_sources:.cpp=.o)))
vkhc_include:= -I $(vkhc_folder)

## ILC objects variables

ilc_sources:= $(wildcard $(ilc_folder)/*.cpp)
ilc_headers:= $(wildcard $(ilc_folder)/*.h)
ilc_objs:=    $(addprefix $(objs_dir)/ilc/, $(notdir $(ilc_sources:.cpp=.o)))
ilc_include:= -I $(ilc_folder)

## Vulkan related variables
vulkan_libs:= -lshaderc -lvulkan 

## GLFW definitions 
glfw_libs:= -lglfw

## IMGUI variables
## (repository must be cloned on $(imgui_git) folder)

imgui_include:= -I $(imgui_git)  -I $(imgui_git)/backends
imgui_src_names_1:=  imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_tables.cpp 
imgui_src_names_2:=imgui_impl_glfw.cpp imgui_impl_vulkan.cpp
imgui_src_1:= $(addprefix $(imgui_git), $(imgui_src_names_1))
imgui_src_2:= $(addprefix $(imgui_git), $(imgui_src_names_2))
imgui_objs_1:= $(addprefix $(objs_dir)/imgui/, $(notdir $(imgui_src_names_1:.cpp=.o)))
imgui_objs_2:= $(addprefix $(objs_dir)/imguibe/, $(notdir $(imgui_src_names_2:.cpp=.o)))
imgui_objs:=  $(imgui_objs_1) $(imgui_objs_2)

## include options
opcs_incl:= $(imgui_include) $(vkhc_include) $(ilc_include) $(app_include) $(stb_include)

## objects to compile
objs:= $(app_objs) $(vkhc_objs) $(ilc_objs) $(imgui_objs) 

## libs to link 
libs:= $(glfw_libs) $(vulkan_libs) -ldl -lpthread

## options for compile and link
compile_flags:= $(compile_options) $(opcs_incl)
link_flags:= $(link_options) 


## ---------------------------------------------------------------------------------
## TARGETS 

## main target
x: $(target)
	./$<

## compile an unit from the IMGUI folder onto 'objs/imgui' folder
$(objs_dir)/imgui/%.o: $(imgui_git)/%.cpp makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## compile an unit from the IMGUI backends folder onto 'objs/imguibe' folder
$(objs_dir)/imguibe/%.o: $(imgui_git)/backends/%.cpp makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## compile an unit from the VKHC folder onto 'objs/vkhc' folder
$(objs_dir)/vkhc/%.o: $(vkhc_folder)/%.cpp $(vkhc_headers) makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## compile an unit from the ILC folder onto 'objs/ilc' folder
$(objs_dir)/ilc/%.o: $(ilc_folder)/%.cpp $(ilc_headers) $(vkhc_headers) makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## compile an unit from the APP folder onto 'objs/$(target)' folder
$(objs_dir)/$(target_base)/%.o: $(app_src_folder)/%.cpp makefile 
	$(compiler) $(compile_flags) -c $< -o $@

## link app
$(target): $(objs) makefile 
	$(compiler) $(link_flags) -o $@ $(objs) $(libs) 


clean:
	rm -f *_exe $(objs_dir)/*/*.o

 


