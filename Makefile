# source files.
SRC =   ./src/Xrender.cpp $\
		./src/ObjectCreators.cpp $\
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
		./src/gui/imgui.cpp $\
		./src/gui/imgui_tables.cpp $\
		./src/gui/imgui_widgets.cpp $\
		./src/gui/imgui_draw.cpp $\
		./src/gui/imgui_demo.cpp $\
		./src/gui/ImGuiFileDialog.cpp $\
		./src/gui/TextEditor.cpp $\
		./src/stk500/stk500.cpp $\
		./src/hex/hex.cpp $\
		./src/gui/imgui_impl_opengl2.cpp $\
		./src/gui/imgui_impl_glfw.cpp $\
		
OBJ = $(SRC:.cpp=.o)
 
OUT = ./lib/libXrender.a
 
# include directories
INCLUDES = -I/usr/local/include -I./src/
 
# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = `pkg-config --cflags glfw3`
 
# compiler
CCC = g++ -std=gnu++17
 
# library paths
LIBS = -L../ -L/usr/local/lib -lm `pkg-config --libs glfw3`
 
# compile flags
LDFLAGS =
 
.SUFFIXES: .cpp
 
default: dep $(OUT)
 
.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@
 
$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)
 
depend: dep
 
dep:
	$(CCC) -M $(CFLAGS) $(LIBS) $(CCFLAGS) $(INCLUDES) $(SRC)
 
clean:
	rm -f $(OBJ) $(OUT) Makefile.bak