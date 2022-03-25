#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "object.h"



void unload_object(OBJECT obj) {
	if(obj != NULL) {
		if(obj->texture_data != NULL) {
			free(obj->texture_data);
			obj->loaded = 0;
		}
	}
}

int load_object_mem(OBJECT obj, char* data, u64 size) {
	int res = 0;

	if(obj != NULL) {
		
		if((obj->texture_data = malloc(size))) {
			if(memmove(obj->texture_data, data, size)) {
				obj->loaded = 1;
				res = 1;
			
				obj->texture_size = size;
				obj->texture_pixels = size/TEXTURE_PIXEL_LENGTH;
			}
		}
	}

	if(res <= 0) {
		res = errno;
	}

	return res;
}


