#!/bin/bash
g++ -std=c++17 test1.cpp -o test1 `pkg-config --cflags glfw3` -I ../src -L../lib/ -lXrender `pkg-config --libs glfw3`  -framework IOKit -framework Cocoa -framework OpenGL
#g++ -std=c++17 png-test.cpp -o png `pkg-config --cflags sdl2` -I ../src -L../lib/ -lXrender `pkg-config --libs sdl2` -framework IOKit -framework Cocoa -framework OpenGL