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
 
OBJ = $(SRC:.cpp=.o)
 
OUT = ./lib/libXrender.a
 
# include directories
INCLUDES = -I/usr/local/include -I./src/
 
# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -g `pkg-config --cflags sdl2`
 
# compiler
CCC = g++ -std=c++14
 
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
	g++ -M $(CFLAGS) $(LIBS) $(CCFLAGS) $(INCLUDES) $(SRC)
 
clean:
	rm -f $(OBJ) $(OUT) Makefile.bak