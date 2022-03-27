#include <sys/unistd.h>

#include "pixel_engine.h"
#include "utils.h"



//#define DEBUG
#define _SSTRCTL sizeof(struct light_t)

static GLFWwindow* window = NULL;
static STATE g_st = NULL;

static int color_uniform = 0;
static int position_uniform = 0;
static int size_uniform = 0;
static int always_visible_uniform = 0;

static u32 light_ubo = 0;
static u64 light_ubo_size = 0;
static u64 min_access_index = 0;
static u64 max_access_index = 0;

static float g_use_rgb[3];



void use_color(u8 r, u8 g, u8 b) {
	g_use_rgb[0] = (float)r/15.0;
	g_use_rgb[1] = (float)g/15.0;
	g_use_rgb[2] = (float)b/15.0;
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

PARTICLE_SYSTEM create_particle_system(
		u32 particle_count, u8 can_die, void(*update_callback)(PARTICLE p, STATE st)) {

	PARTICLE_SYSTEM system = NULL;

	if(update_callback == NULL) {
		fprintf(stderr, "particle systems require callback, its useless now. :(\n");
		goto give_up;
	}

	if((system = malloc(sizeof *system))) {
		
		system->mem_length = sizeof *system->particles*particle_count;
		system->particles = NULL;

		if((system->particles = malloc(system->mem_length))) {
			
			system->particle_count = particle_count;
			system->can_die   = can_die;
			system->always_visible = 1;
			system->last_dead = 0;
			system->u[0] = 0;
			system->u[1] = 0;
			system->update_callback = update_callback;


			for(u32 i = 0; i < particle_count; i++) {
				PARTICLE p = &system->particles[i];
				p->rgb[0] = 16;
				p->rgb[1] = 16;
				p->rgb[2] = 16;
				p->index = i;
				p->dead  = 1;
				
				p->x = 0.0;
				p->y = 0.0;
				p->vx = 0.0;
				p->vy = 0.0;
				p->ax = 0.0;
				p->ay = 0.0;
				p->lifetime = 0.0;
				p->max_lifetime = 0.0;
			}

		}


	}

give_up:
	return system;
}

void destroy_particle_system(PARTICLE_SYSTEM system) {
	if(system != NULL) {
		if(system->particles != NULL) {
			free(system->particles);
		}
		free(system);
	}
}

void update_particles(PARTICLE_SYSTEM system) {
	if(system != NULL && system->particles != NULL) {
		PARTICLE p = NULL;

		for(u32 i = 0; i < system->particle_count; i++) {
			p = &system->particles[i];

			if(!p->dead) {
				if(system->can_die) {
					if(p->max_lifetime > 0.0) {
						if(p->lifetime > p->max_lifetime) {
							p->lifetime = 0.0;
							p->dead = 1;
							system->last_dead = i;
							continue;
						}
					}
				}
				p->lifetime += g_st->dt;
			}

			system->update_callback(p, g_st);
		}
	}

}

void init_engine(char* title, u16 width, u16 height, int flags) {
	
	window = NULL;
	g_st = NULL;
	color_uniform = 0;
	position_uniform = 0;
	size_uniform = 0;
	always_visible_uniform = 0;
	light_ubo = 0;
	light_ubo_size = 0;

	min_access_index = 0;
	max_access_index = 0;

	g_use_rgb[0] = 0.0;
	g_use_rgb[1] = 0.0;
	g_use_rgb[2] = 0.0;

	if(!(g_st = malloc(sizeof *g_st))) {
		fprintf(stderr, "Failed to allocate memory for g_state_t!\n");
		return;
	}

	g_st->flags = 0;
	g_st->time = 0.0;
	g_st->vao = 0;
	g_st->vbo = 0;

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

	glfwWindowHint(GLFW_RED_BITS, 2);
	glfwWindowHint(GLFW_GREEN_BITS, 2);
	glfwWindowHint(GLFW_BLUE_BITS, 2);
	glfwWindowHint(GLFW_REFRESH_RATE, 60);

	int mon_x = 0;
	int mon_y = 0;
	int mon_w = 0;
	int mon_h = 0;

	glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &mon_x, &mon_y, &mon_w, &mon_h);
	
	if(flags & FLG_INIT_FULLSCREEN) {	
		#ifdef DEBUG
		mon_w/=2;
		mon_h/=2;
		mon_x += mon_w;
		#endif

	}
	else {
		mon_x = mon_w/2-(width/2);
		mon_y = mon_h/2-(height/2);
		mon_w = width;
		mon_h = height;
	}

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



	g_st->num_pixels = g_st->max_col*g_st->max_row;
	const u32 stride = sizeof(float)*5;
	g_st->buffer_length = stride*g_st->num_pixels;
	g_st->buffer = NULL;
	//printf("%p, %li\n", pixels, vbo_size);
	
	glGenVertexArrays(1, &g_st->vao);
	glBindVertexArray(g_st->vao);

	glGenBuffers(1, &g_st->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_st->vbo);

	glBufferData(GL_ARRAY_BUFFER, g_st->buffer_length, NULL, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)*2));
	glEnableVertexAttribArray(1);


	entity_set_next_id(0);
	g_st->max_fps = 60;
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


void start_engine(void(*callback)(STATE), void(*start_callback)(STATE)) {
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
		"in vec3 f_col;"

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
			"vec3 col = f_col;"
			
			"gl_FragColor = vec4(col, 1.0);"

			/*
			"float l = distance(pos, lights[0].pos);"
			"l = floor(l*25.0)*0.2;"
			"float i = BRIGHTNESS/(1.0+L*l+Q*pow(l,2.0));"
			
			"vec3 lcolor = vec3(1.0, 0.7, 0.56);"

			"gl_FragColor = vec4(col*i, 1.0);"
			*/
		"}";



	const char* vertex_source =
		"#version 330\n"
		"layout(location = 0) in vec2 i_pos;"
		"layout(location = 1) in vec3 i_col;"
		
		"out vec2 f_pos;"
		"out vec3 f_col;"

		"void main() {"
			"gl_Position = vec4(i_pos.x, i_pos.y, 0.0, 1.0);"
			"f_pos = i_pos;"
			"f_col = i_col;"
		"}"
		;

	const u32 program = create_shader(vertex_source, fragment_source);

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

	if(start_callback != NULL) {
		start_callback(g_st);
	}


	glBindBuffer(GL_ARRAY_BUFFER, g_st->vbo);
	
	const u32 off = sizeof(float)*5;


	while(!glfwWindowShouldClose(window)) {
		g_st->time = glfwGetTime();

		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
	
		g_st->buffer = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	
		if(g_st->buffer != NULL) {
			memset(g_st->buffer, -2, g_st->buffer_length);

			glUseProgram(program);
			callback(g_st);

			glUnmapBuffer(GL_ARRAY_BUFFER);
			g_st->buffer = NULL;

			glBindVertexArray(g_st->vao);

			glDrawArrays(GL_POINTS, 0, g_st->num_pixels);
		}
	
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
		glDeleteVertexArrays(1, &g_st->vao);
		glDeleteBuffers(1, &g_st->vbo);
		
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

void draw_pixel(u32 x, u32 y) {
	u32 i = (y*g_st->max_col+x);
	
	if(i < g_st->num_pixels) {
		i *= 5;

		g_st->buffer[i]   = map(x, 0, g_st->max_col, -1.0,  1.0);
		g_st->buffer[i+1] = map(y, 0, g_st->max_row,  1.0, -1.0);
		g_st->buffer[i+2] = g_use_rgb[0];
		g_st->buffer[i+3] = g_use_rgb[1];
		g_st->buffer[i+4] = g_use_rgb[2];

		g_st->flags |= FLG_VBO_UPDATE;
	}

}


/*

void draw_area(int x, int y, int w, int h, u8 always_visible) {
	if(w > 0 && h > 0) {
		glUniform1i(always_visible_uniform, always_visible);
		_draw_f(x, y, w*2, h*2);
	}
}

void draw_box_outline(int x, int y, int w, int h, u8 always_visible) {
	draw_area(x, y, w, 1, always_visible);
	draw_area(x, y+h, w, 1, always_visible);
	draw_area(x, y, 1, h, always_visible);
	draw_area(x+w, y, 1, h+1, always_visible);
}


*/
void draw_object(OBJECT obj, u32 x, u32 y) {
	if(obj != NULL && obj->texture_data != NULL && obj->loaded) {
		u32 p = 0;
		for(u32 i = 0; i < obj->texture_pixels; i++) {
			use_color(
					obj->texture_data[p+2],
					obj->texture_data[p+3],
					obj->texture_data[p+4]
					);
			draw_pixel(
					x+obj->texture_data[p],
				   	y+obj->texture_data[p+1]);

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
		draw_pixel(x0, y0);
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
u64 grid_index(u32 x, u32 y) {
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


