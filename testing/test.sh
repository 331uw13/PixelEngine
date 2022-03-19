#!/bin/bash

if (cd .. && ./build.sh); then
	if gcc main.c weapon.c enemy.c -L/home/paavo/Dev/PixelEngine -lpixel_engine -lGL -lglfw -lGLEW -lSDL2 -lSDL2_mixer -ldl -lm; then
		./a.out
	fi
fi




