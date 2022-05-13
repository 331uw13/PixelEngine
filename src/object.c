#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "object.h"
#include "utils.h"


OBJECT create_object(char* data, u64 size) {
	OBJECT obj = malloc(sizeof *obj);

	if(obj != NULL) {
		obj->texture_size = 0;
		obj->texture_pixels = 0;
		obj->flags = 0;
		obj->blink = 0.0;
		obj->shake = 0.0;
		obj->shake_x = 2;
		obj->shake_y = 1;
		obj->blink_rate = 0.135;
		obj->texture_data = malloc(size);
		if(obj->texture_data != NULL) {
			memmove(obj->texture_data, data, size);
			obj->texture_size = size;
			obj->texture_pixels = size/TEXTURE_PIXEL_LENGTH;
			obj->flags |= OBJ_FLG_LOADED;
			obj->flags |= OBJ_FLG_VISIBLE;
		}

	}

	return obj;
}

void destroy_object(OBJECT obj) {
	if(obj != NULL) {
		obj->flags = 0;
		if(obj->texture_data != NULL) {
			free(obj->texture_data);
		}
		free(obj);
	}
}

void object_set_color(OBJECT obj, u32 start, u32 end, char r, char g, char b) {
	u32 j = start;
	for(u32 i = start; i < end; i++) {
		obj->texture_data[j+2] = r;
		obj->texture_data[j+3] = g;
		obj->texture_data[j+4] = b;
		j += 5;
	}
}

void object_add_color(OBJECT obj, u32 start, u32 end, char r, char g, char b) {
	u32 j = start;
	for(u32 i = start; i < end; i++) {
		char* ptr_r = &obj->texture_data[j+2];
		char* ptr_g = &obj->texture_data[j+3];
		char* ptr_b = &obj->texture_data[j+4];

		*ptr_r = CLAMP_VALUE((*ptr_r)+r, 0, 16);
		*ptr_g = CLAMP_VALUE((*ptr_g)+g, 0, 16);
		*ptr_b = CLAMP_VALUE((*ptr_b)+b, 0, 16);
		
		j += 5;
	}
}

void object_flip_x(OBJECT obj) {
	for(u32 i = 0; i < obj->texture_pixels; i++) {
		

	}
}


