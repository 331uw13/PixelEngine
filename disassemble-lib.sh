#!/bin/bash

objdump -d libpixel_engine.a -M intel --visualize-jumps=color | less -r

