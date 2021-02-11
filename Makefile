# source files.
SRC =  ./src/Xrender.cpp $\
		./src/ObjectCreators.cpp $\
		./src/EventCreators.cpp $\
		./src/Debug.cpp $\
		./src/Timers.cpp $\
		./src/AppDirs.cpp $\
		./src/dxflib/dl_dxf.cpp $\
		./src/dxflib/dl_writer_ascii.cpp $\
		./src/dxf/DXFParse_Class.cpp $\
		./src/Dxf.cpp $\
		./src/dxf/Bezier.cpp $\
		./src/geometry/clipper.cpp $\
		./src/geometry/geometry.cpp $\
		./src/dxf/Curve.cpp $\
		./src/dxf/Vector.cpp $\
		./src/serial/serial.cpp $\
		./src/serial/impl/unix.cpp $\
		./src/serial/impl/win.cpp $\
		./src/serial/impl/list_ports/list_ports_linux.cpp $\
		./src/serial/impl/list_ports/list_ports_osx.cpp $\
		./src/serial/impl/list_ports/list_ports_win.cpp $\
		./src/gui/imgui_impl_sdl.cpp $\
		./src/gui/imgui.cpp $\
		./src/gui/imgui_tables.cpp $\
		./src/gui/imgui_widgets.cpp $\
		./src/gui/imgui_draw.cpp $\
		./src/gui/imgui_demo.cpp $\
		./src/gui/imgui_sdl.cpp $\
		./src/gui/ImGuiFileDialog.cpp $\
 
OBJ = $(SRC:.cpp=.o)
 
OUT = ./lib/libXrender.a
 
# include directories
INCLUDES = -I/usr/local/include -I./src/
 
# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -g `pkg-config --cflags sdl2`
 
# compiler
CCC = g++ -std=c++17 -g
 
# library paths
LIBS = -L../ -L/usr/local/lib -lm `pkg-config --libs sdl2` -lSDL2_image -lSDL2_ttf -lSDL2_gfx
 
# compile flags
LDFLAGS = -g
 
.SUFFIXES: .cpp
 
default: dep $(OUT)
 
.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@
 
$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)
 
depend: dep
 
dep:
	g++ -std=c++17 -M $(CFLAGS) $(LIBS) $(CCFLAGS) $(INCLUDES) $(SRC)
 
clean:
	rm -f $(OBJ) $(OUT) Makefile.bak