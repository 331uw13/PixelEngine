#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"

#define OBJ_FLG_LOADED   0x1
#define OBJ_FLG_VISIBLE  0x4

struct object_t {
	char*  texture_data;
	u64   texture_size;
	u32   texture_pixels;
	
	double  blink;
	double  shake;
	double  counter;
	double  blink_rate;
	int     shake_x;
	int     shake_y;
	int     flags;
};

#define OBJECT struct object_t*


OBJECT create_object(char* data, u64 size);
OBJECT create_object_from_file(char* filename);
void   destroy_object(OBJECT obj);

void   object_set_color(OBJECT obj, u32 start, u32 end, char r, char g, char b);
void   object_add_color(OBJECT obj, u32 start, u32 end, char r, char g, char b);


#endif
