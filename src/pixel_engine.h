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
#include "object.h"

#define MAX_LIGHTS 8
#define LIGHT_UBO_BINDING 1
#define RANDOM_GEN_MAX 0x7FFF

#define FLG_MOUSE_LDOWN (1<<1)
#define FLG_MOUSE_RDOWN (1<<2)

/*


TODO:

	!!!!    FIX LIGHTS


	*  Multiple light sources.
	*  Useful functions like: random value, color fade, line, circle, 
             check if position is inside some region (rect or circle)


*/

struct light_t {
	float brightness;
	float x;
	float y;
	float _reserved;
};

struct particle_t {
	u8 rgb[3];
	float x;
	float y;
	float ax;
	float ay;
	float vx;
	float vy;
	float lifetime;
	float max_lifetime;
};

struct particle_system_t {
	u64 mem_length;
	struct particle_t* particles;
	u32 count;

	void(*update_callback)(struct particle_t* p);
	void(*death_callback)(struct particle_t* p);

	u32    max_count;
	float  max_lifetime;
	u8     always_visible;
	u8     can_die;
};

struct g_state_t {
	int      flags;
	int      seed;
	double   time;
	double   dt;    // delta time
	u8       active_light_count;

	int      window_width;
	int      window_height;
	int      max_row;
	int      max_col;

	char* grid;
	u64 grid_length;
	
	struct light_t lights[MAX_LIGHTS];
};


void shutdown_engine();
void init_engine(char* title);
void start_engine(void(*callback)(struct g_state_t*));

void mouse_pos(u16* x, u16* y);
void normal_mouse_pos(float* x, float* y);

float normalize(float t, float min, float max);
float lerp(float t, float min, float max);
float map(float t, float s_min, float s_max, float d_min, float d_max);

int   random_gen();
int   randomi(int min, int max);
float randomf(float min, float max);

void update_light(u32 index);
void update_lights();

void _draw_f(float x, float y, float w, float h);

// TODO: change all dimensions to integer type

void draw_pixel(int x, int y, u8 always_visible);
void draw_area(int x, int y, int w, int h, u8 always_visible);
void draw_object(struct object_t* obj, u8 always_visible);
void draw_line(u16 x0, u16 y0, u16 x1, u16 y1, u8 always_visible);
void draw_box_outline(int x, int y, int w, int h, u8 always_visible);


int  create_particle_system(
		u32 particle_count,
		void(*update_callback)(struct particle_t* p),
		void(*death_callback)(struct particle_t* p),
		struct particle_system_t* system
		);

void destroy_particle_system(struct particle_system_t* system);
void update_particles(struct particle_system_t* system);

int ray_cast(int src_x, int src_y, int dst_x, int dst_y, int* hit_x, int* hit_y);
u64 grid_index(int x, int y);

u8 inside_rect(int rect_x, int rect_y, int rect_w, int rect_h, int x, int y);

void use_color(u8 r, u8 g, u8 b);
void back_color(u8 r, u8 g, u8 b);

GLFWwindow* engine_win();








#endif


