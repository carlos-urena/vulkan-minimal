.SUFFIXES: 
.PHONY: x clean 

app_src_folder:=  ../src/app-2d## folder with sources for the test application
target_base:=     app-2d

include include.make

## clean all compilation files
clean:
	rm -f ./bin/$(target_base)_exe ./objs/$(target_base)/*.o