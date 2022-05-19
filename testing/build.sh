#!/bin/bash

if (cd .. && make); then
	if gcc $1  -L../ -lpixel_engine -lGL -lglfw -lGLEW -ldl -lm -lz; then
		#echo "ok"
		./a.out $2
	fi
fi




