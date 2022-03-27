#ifndef PIXEL_ENGINE_H
#define PIXEL_ENGINE_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <time.h>

#include "shader.h"
#include "entity.h"
#include "state.h"

#define LIGHT_UBO_BINDING 1


/*


TODO:

	!!!!    FIX LIGHTS


	*  Multiple light sources.
	*  Useful functions like: random value, color fade, line, circle, 
             check if position is inside some region (rect or circle)


*/

#define PARTICLE_SYSTEM struct particle_system_t*
#define PARTICLE        struct particle_t*

struct particle_t {
	u8 rgb[3];
	u8 dead;
	u32 index;
	float x;
	float y;
	float ax;
	float ay;
	float vx;
	float vy;
	float lifetime;
	float max_lifetime;
	
	u16 u[2]; // for user
};

struct particle_system_t {
	PARTICLE particles;
	u64      mem_length;
	u32      last_dead;
	u32      particle_count;

	u8     always_visible;
	u8     can_die;
	
	void(*update_callback)(PARTICLE p, STATE st);
	
	u16 u[2];  // for user
};



void shutdown_engine();
void init_engine(char* title, u16 width, u16 height, int flags);
void start_engine(void(*callback)(STATE), void(*start_callback)(STATE));

void update_light(u32 index);
void update_lights();

void _draw_f(float x, float y, float w, float h);

// TODO: change all dimensions to integer type

void draw_pixel(u32 x, u32 y);
void draw_object(OBJECT obj, u32 x, u32 y);

/*
void draw_area(int x, int y, int w, int h, u8 always_visible);
void draw_line(u16 x0, u16 y0, u16 x1, u16 y1, u8 always_visible);
void draw_box_outline(int x, int y, int w, int h, u8 always_visible);
*/

PARTICLE_SYSTEM create_particle_system(
		u32 particle_count,
		u8  can_die,
		void(*update_callback)(PARTICLE p, STATE st));

void destroy_particle_system(PARTICLE_SYSTEM system);
void update_particles(PARTICLE_SYSTEM system);

int ray_cast(int src_x, int src_y, int dst_x, int dst_y, int* hit_x, int* hit_y);
u64 grid_index(u32 x, u32 y);

u8 inside_rect(int rect_x, int rect_y, int rect_w, int rect_h, int x, int y);

void use_color(u8 r, u8 g, u8 b);
void back_color(u8 r, u8 g, u8 b);

GLFWwindow* engine_win();

#endif

