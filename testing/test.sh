#!/bin/bash

if (cd .. && make); then
	if gcc main.c  -L../ -lpixel_engine -lGL -lglfw -lGLEW -lSDL2 -lSDL2_mixer -ldl -lm; then
		./a.out
	fi
fi




