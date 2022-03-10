#ifndef PIXEL_ENGINE_SHADER_H
#define PIXEL_ENGINE_SHADER_h

#include "types.h"

#define SHADER_MODULE 0
#define SHADER_PROGRAM 1

u8 plx_shader_ok(u32 id, u32 type);
u32 plx_create_shader(const char* vssrc, const char* fssrc);


#endif
