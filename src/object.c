#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "object.h"



void unload_object(OBJECT obj) {
	if(obj != NULL) {
		if(obj->texture_data != NULL) {
			free(obj->texture_data);
		}
		free(obj);
	}
}

OBJECT load_object_mem(char* data, u64 size) {
	OBJECT obj = malloc(sizeof *obj);

	if(obj != NULL) {
		obj->loaded = 0;
		obj->texture_size = 0;
		obj->texture_pixels = 0;
		obj->texture_data = malloc(size);
		if(obj->texture_data != NULL) {
			memmove(obj->texture_data, data, size);
			obj->texture_size = size;
			obj->texture_pixels = size/TEXTURE_PIXEL_LENGTH;
			obj->loaded = 1;
		}

	}



	return obj;
}


