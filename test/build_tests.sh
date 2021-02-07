#!/bin/bash
g++ -std=c++17 test1.cpp -o test1 `pkg-config --cflags sdl2` -I ../src -L../lib/ -lXrender `pkg-config --libs sdl2` -lSDL2_image -lSDL2_ttf -lSDL2_gfx
