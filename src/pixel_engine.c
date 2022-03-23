#include <sys/unistd.h>

#include "pixel_engine.h"
#include "utils.h"



//#define DEBUG
#define _SSTRCTL sizeof(struct light_t)

static GLFWwindow* window = NULL;
static struct g_state_t* g_st = NULL;

static int color_uniform = 0;
static int position_uniform = 0;
static int size_uniform = 0;
static int always_visible_uniform = 0;

static u32 light_ubo = 0;
static u64 light_ubo_size = 0;


void use_color(u8 r, u8 g, u8 b) {
	glUniform3f(color_uniform, (float)r/15.0, (float)g/15.0, (float)b/15.0);
}

void back_color(u8 r, u8 g, u8 b) {
	glClearColor((float)r/15.0, (float)g/15.0, (float)b/15.0, 1.0);
}




void update_light(u32 index) {
	if(index < MAX_LIGHTS) {
		if(light_ubo != 0 && light_ubo_size != 0) {

			// NOTE: binding and unbinding this buffer object may not be needed everytime...
			glBindBuffer(GL_UNIFORM_BUFFER, light_ubo);
			
			glBufferSubData(GL_UNIFORM_BUFFER, _SSTRCTL*index, _SSTRCTL, &g_st->lights[index]);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
	}
}

void update_lights() {
	glBufferData(GL_UNIFORM_BUFFER, light_ubo_size, g_st->lights, GL_STREAM_DRAW);
}


int create_particle_system(
		u32 particle_count,
		u8  can_die,
		void(*update_callback)(struct particle_t* p, struct g_state_t* st),
		struct particle_system_t* system) {
	int res = 0;


	if(system != NULL && update_callback != NULL) {

		system->mem_length = particle_count*sizeof *system->particles;
		if(!(system->particles = malloc(system->mem_length))) {
			fprintf(stderr, "failed to allocate memory for particles, tried to allocate %li bytes of memory!\n",
						system->mem_length);
			res = errno;
			goto failed;
		}

		for(u32 i = 0; i < particle_count; i++) {
			system->particles[i].dead = can_die;
			system->particles[i].index = i;
		}

		system->count = particle_count;
		system->can_die = can_die;
		system->update_callback = update_callback;
		system->always_visible = 1;
		system->last_dead = 0;
	}

failed:
	return res;
}

void destroy_particle_system(struct particle_system_t* system) {
	if(system != NULL) {
		free(system->particles);
	}
}

void update_particles(struct particle_system_t* system) {
	
	if(system != NULL && system->particles != NULL) {
		struct particle_t* p = NULL;

		for(u32 i = 0; i < system->count; i++) {
			p = &system->particles[i];

			if(!p->dead) {
				if(system->can_die) {
					if(p->max_lifetime > 0.0) {
						if(p->lifetime > p->max_lifetime) {
							p->lifetime = 0.0;
							p->max_lifetime = system->max_lifetime;
							p->dead = 1;
							system->last_dead = i;
							continue;
						}
					}
				}
				p->lifetime += g_st->dt;
			}


			system->update_callback(p, g_st);
		
			if(!p->dead) {
				draw_pixel(p->x, p->y, system->always_visible);
			}
		}
	}

}


void mouse_pos(u16* x, u16* y) {
	double xd = 0;
	double yd = 0;
	glfwGetCursorPos(window, &xd, &yd);
	
	if(x != NULL) {
		*x = (u16)xd/PIXEL_SIZE;
	}
	if(y != NULL) {
		*y = (u16)yd/PIXEL_SIZE;
	}
}

void normal_mouse_pos(float* x, float* y) {
	double xd = 0;
	double yd = 0;
	glfwGetCursorPos(window, &xd, &yd);


	if(x != NULL) {
		*x = map(xd, 0.0, g_st->window_width, -1.0, 1.0);
	}

	if(y != NULL) {
		*y = map(yd, 0.0, g_st->window_height, 1.0, -1.0);
	}
}


void init_engine(char* title) {
	
	window = NULL;
	g_st = NULL;
	color_uniform = 0;
	position_uniform = 0;
	size_uniform = 0;
	always_visible_uniform = 0;
	light_ubo = 0;
	light_ubo_size = 0;
	
	if(!(g_st = malloc(sizeof *g_st))) {
		fprintf(stderr, "Failed to allocate memory for g_state_t!\n");
		return;
	}

	g_st->flags = 0;
	g_st->time = 0.0;
	set_seed(time(0));

	if(!glfwInit()) {
		fprintf(stderr, "glfw failed!\n");
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);

	int mon_x = 0;
	int mon_y = 0;
	int mon_w = 0;
	int mon_h = 0;

	glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &mon_x, &mon_y, &mon_w, &mon_h);

#ifdef DEBUG
	mon_w/=2;
	mon_h/=2;
	mon_x += mon_w;
#endif

	window = glfwCreateWindow(mon_w, mon_h, title, NULL, NULL);
	if(window == NULL) {
		fprintf(stderr, "failed to create window!\n");
		shutdown_engine();
		return;
	}
	
	glfwSetWindowPos(window, mon_x, mon_y);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwGetWindowSize(window, &g_st->window_width, &g_st->window_height);

	if(g_st->window_width*g_st->window_height <= 0) {
		fprintf(stderr, "glfwGetWindowSize returned width:%i, height:%i\n", g_st->window_width, g_st->window_height);
		shutdown_engine();
		return;
	}

	g_st->max_col = g_st->window_width/PIXEL_SIZE;
	g_st->max_row = g_st->window_height/PIXEL_SIZE;

#ifndef DEBUG
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
	if(glewInit() != GLEW_OK) {
		fprintf(stderr, "glew failed!\n");
		shutdown_engine();
		return;
	}


	g_st->grid_length = sizeof *g_st->grid*(g_st->max_row*g_st->max_col);
	if(!(g_st->grid = malloc(g_st->grid_length))) {
		fprintf(stderr, "failed to allocate memory for grid! tried to allocate %li bytes of memory\n", g_st->grid_length);
		shutdown_engine();
		return;
	}
	
	memset(g_st->grid, 0, g_st->grid_length);

	light_ubo_size = MAX_LIGHTS*sizeof *g_st->lights;

	glGenBuffers(1, &light_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, light_ubo);
	glBufferData(GL_UNIFORM_BUFFER, light_ubo_size, NULL, GL_STREAM_DRAW);
	
	glBindBufferRange(GL_UNIFORM_BUFFER, LIGHT_UBO_BINDING, light_ubo, 0, light_ubo_size);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	g_st->max_fps = 60.0;
}

void start_engine(void(*callback)(struct g_state_t*)) {
	const char* fragment_source = 
		"#version 330\n"
		"#ifdef GL_ARB_shading_language_420pack\n"
		"#extension GL_ARB_shading_language_420pack : require\n"
		"#endif\n"
		
		"uniform vec3 color;"
		"uniform vec2 pos;"
		"uniform vec2 size;"
		"uniform int  always_visible;" //  !!! TODO: remove branching from shaders!

		"in vec2 f_pos;"

		"struct light_t {"
			"float brightness;"
			"vec2 pos;"
			"float _r;"
		"};"

		"\n"
		
		"layout(std140, binding = 1) uniform light_array {"
			"light_t lights[8];\n"
		"};\n"


		// TODO: make lights better (later..)

		"\n"
		"#define BRIGHTNESS 0.8\n"
		"#define L 0.6\n"
		"#define Q 0.5\n"

		"void main() {"
			"if(always_visible > 0) {"
				"gl_FragColor = vec4(color, 1.0);"
				"return;"
			"}"

			"float l = distance(pos, lights[0].pos);"
			"l = floor(l*25.0)*0.2;"
			"float i = BRIGHTNESS/(1.0+L*l+Q*pow(l,2.0));"
			
			"vec3 lcolor = vec3(1.0, 0.7, 0.56);"

			"gl_FragColor = vec4(color*i, 1.0);"
		"}";

	const char* vertex_source =
		"#version 330\n"
		"layout(location = 0) in vec2 i_pos;"
		"out vec2 f_pos;"
		"void main() {"
			"gl_Position = vec4(i_pos.x, i_pos.y, 0.0, 1.0);"
			"f_pos = i_pos;"
		"}"
		;

	const u32 program = plx_create_shader(vertex_source, fragment_source);

	glPointSize(PIXEL_SIZE);
	
	glUseProgram(program);
	color_uniform      = glGetUniformLocation(program, "color");
	position_uniform   = glGetUniformLocation(program, "pos");
	size_uniform       = glGetUniformLocation(program, "size");
	always_visible_uniform  = glGetUniformLocation(program, "always_visible");

	for(int i = 0; i < MAX_LIGHTS; i++) {
		g_st->lights[i].brightness = 0.0;
		g_st->lights[i].x = 0.0;
		g_st->lights[i].y = 0.0;
		g_st->lights[i]._reserved = 0.0;
		update_light(i);
	}


	int frames = 0;
	double second_counter = 0.0;


	while(!glfwWindowShouldClose(window)) {
		g_st->time = glfwGetTime();

		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program);


		callback(g_st);

		glfwSwapBuffers(window);
		g_st->dt = glfwGetTime()-g_st->time;
		
		second_counter += glfwGetTime()-g_st->time;
		frames++;

		if(second_counter >= 1.0) {
			g_st->fps = ((double)frames)*0.5+g_st->max_fps*0.5;
			second_counter = 0.0;

			frames = 0;

		}

	}
	
	glDeleteProgram(program);
}

void shutdown_engine() {
	
	if(g_st != NULL) {
		free(g_st->grid);
		free(g_st);
	}
	
	if(window != NULL) {
		if(light_ubo > 0) {
			glDeleteBuffers(1, &light_ubo);
		}
		glfwDestroyWindow(window);
	}
	
	glfwTerminate();
	puts("exit.");
	exit(0);
}


void _draw_f(float x, float y, float w, float h) {

	// TODO: FIX THIS!

	float rx = map(x*PIXEL_SIZE, 0.0, g_st->window_width,  -1.0,  1.0);
	float ry = map(y*PIXEL_SIZE, 0.0, g_st->window_height,  1.0, -1.0);
	
	rx += PIXEL_SIZE/(float)g_st->window_width;
	ry += PIXEL_SIZE/(float)g_st->window_height;

	const float rw = w/g_st->max_col;
	const float rh = h/g_st->max_row;
	
	glUniform2f(size_uniform, rw, rh);
	glUniform2f(position_uniform, rx, ry);
	
	glBegin(GL_QUADS);
	glVertex2f(rx, ry);
	glVertex2f(rx+rw, ry);
	glVertex2f(rx+rw, ry-rh);
	glVertex2f(rx, ry-rh);


	glEnd();
	glUniform2f(size_uniform, -1, -1);
}


void draw_pixel(int x, int y, u8 always_visible) {
	float rx = map(x*PIXEL_SIZE, 0.0, g_st->window_width, -1.0, 1.0);
	float ry = map(y*PIXEL_SIZE, 0.0, g_st->window_height, 1.0, -1.0);
	
	glUniform1i(always_visible_uniform, always_visible); // TODO: this is bad, dont do this here
	glUniform2f(position_uniform, rx, ry);
	
	glBegin(GL_POINTS);
	glVertex2f(rx, ry);
	glEnd();
}

void draw_area(int x, int y, int w, int h, u8 always_visible) {
	if(w > 0 && h > 0) {
		glUniform1i(always_visible_uniform, always_visible);
		_draw_f(x, y, w*2, h*2);
	}
}

void draw_object(struct object_t* obj, u8 always_visible) {
	if(obj != NULL && obj->texture_data != NULL && obj->loaded) {
		u32 p = 0;
		for(u32 i = 0; i < obj->texture_pixels; i++) {
			use_color(
					obj->texture_data[p+2],
					obj->texture_data[p+3],
					obj->texture_data[p+4]
					);
			draw_pixel(
					obj->x+obj->texture_data[p],
				   	obj->y+obj->texture_data[p+1], always_visible);

			p += 5;
		}
	}
}

void draw_line(u16 x0, u16 y0, u16 x1, u16 y1, u8 always_visible) {
	int width = x1-x0;
	int height = y1-y0;
	int dx0 = 0;
	int dy0 = 0;
	int dx1 = 0;
	int dy1 = 0;

	dx1 = dx0 = (width < 0) ? -1 : 1;
	dy0 = (height < 0) ? -1 : 1;

	int aw = abs(width);
	int ah = abs(height);
	int longest = aw;
	int shortest = ah;
    
	if(longest < shortest) {
		longest = ah;
		shortest = aw;
		dy1 = (height < 0) ? -1 : 1;
		dx1 = 0;
	}
	
	int numerator = longest >> 1;
	
	for(int i = 0; i < longest; i++) {
		draw_pixel(x0, y0, always_visible);
		numerator += shortest;
		if(numerator > longest) {
			numerator -= longest;
			x0 += dx0;
			y0 += dy0;
		}
		else {
			x0 += dx1;
			y0 += dy1;
		}
	}
	
}

void draw_box_outline(int x, int y, int w, int h, u8 always_visible) {
	draw_area(x, y, w, 1, always_visible);
	draw_area(x, y+h, w, 1, always_visible);
	draw_area(x, y, 1, h, always_visible);
	draw_area(x+w, y, 1, h+1, always_visible);
}

u64 grid_index(int x, int y) {
	const u64 i = y*g_st->max_col+x;
	return (i > g_st->grid_length) ? 0 : i;
}

int ray_cast(int src_x, int src_y, int dst_x, int dst_y, int* hit_x, int* hit_y) {
	int hit = 0;

	int width = dst_x-src_x;
	int height = dst_y-src_y;
	int dx0 = 0;
	int dy0 = 0;
	int dx1 = 0;
	int dy1 = 0;

	dx1 = dx0 = (width < 0) ? -1 : 1;
	dy0 = (height < 0) ? -1 : 1;

	int aw = abs(width);
	int ah = abs(height);
	int longest = aw;
	int shortest = ah;
    
	if(longest < shortest) {
		longest = ah;
		shortest = aw;
		dy1 = (height < 0) ? -1 : 1;
		dx1 = 0;
	}
	
	int numerator = longest >> 1;
	
	for(int i = 0; i < longest; i++) {
		
		if(g_st->grid[grid_index(src_x, src_y)]) {
			hit = 1;
			if(hit_x != NULL) {
				*hit_x = src_x;
			}
			if(hit_y != NULL) {
				*hit_y = src_y;
			}
			break;
		}
		
		numerator += shortest;
		if(numerator > longest) {
			numerator -= longest;
			src_x += dx0;
			src_y += dy0;
		}
		else {
			src_x += dx1;
			src_y += dy1;
		}
	}

	return hit;
}

u8 inside_rect(int rect_x, int rect_y, int rect_w, int rect_h, int x, int y) {
	return (x >= rect_x && y >= rect_y) && (x <= rect_x+rect_w && y <= rect_y+rect_h);
}


GLFWwindow* engine_win() {
	return window;
}


