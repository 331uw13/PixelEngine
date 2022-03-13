#!/bin/bash

if (cd .. && ./build.sh); then
	if gcc main.c weapon.c -L/home/paavo/Dev/PixelEngine -lpixel_engine -lGL -lglfw -lGLEW -ldl -lm; then
		./a.out
	fi
fi




