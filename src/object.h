#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"


/*
	Texture data segment: x, y, r, g, b
*/


struct object_t {
	u8    loaded;
	char*  texture_data;
	u64   texture_size;
	u32   texture_pixels;
	int x;
	int y;
};



void unload_object(struct object_t* obj);

// Create an object from memory
int load_object_mem(struct object_t* obj, char* data, u64 size);


// TODO:
// int load_object_file(struct object_t* obj, u64 size, char* filename);



#endif
