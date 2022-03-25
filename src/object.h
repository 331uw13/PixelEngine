#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"


/*
	Texture data segment: x, y, r, g, b
*/


struct object_t {
	char*  texture_data;
	u64   texture_size;
	u32   texture_pixels;
	u8    loaded;
};


#define OBJECT struct object_t*


void unload_object(OBJECT obj);

// Create an object from memory
int load_object_mem(OBJECT obj, char* data, u64 size);


// TODO:
// int load_object_file(struct object_t* obj, u64 size, char* filename);



#endif
