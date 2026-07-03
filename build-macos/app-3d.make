.SUFFIXES: 
.PHONY: x clean 

app_src_folder:=  ../src/app-3d## folder with sources for the test application
target_base:=     app-3d

include include.make



## clean all compilation files
clean:
	rm -f ./bin/$(target_base)_exe ./objs/$(target_base)/*.o