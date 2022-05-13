#include <sys/unistd.h>
#include <zlib.h>
#include <math.h>

#include "pixel_engine.h"
#include "utils.h"



//#define DEBUG

static GLFWwindow* window = NULL;
static STATE g_st = NULL;

static u64 min_access_index = 0;
static u64 max_access_index = 0;

static float g_use_rgb[3];
static int g_program = 0;


void use_color(u8 r, u8 g, u8 b) {
	g_use_rgb[0] = (float)r/15.0;
	g_use_rgb[1] = (float)g/15.0;
	g_use_rgb[2] = (float)b/15.0;
}

void back_color(u8 r, u8 g, u8 b) {
	glClearColor((float)r/15.0, (float)g/15.0, (float)b/15.0, 1.0);
}

void dim_color(u8 v) {
	float f = v/16.0;
	g_use_rgb[0] -= f;
	g_use_rgb[1] -= f;
	g_use_rgb[2] -= f;
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

void update_particles(PARTICLE_SYSTEM system, u32 num) {
	if(system != NULL && system->particles != NULL) {
		PARTICLE p = NULL;

		for(u32 i = 0; i < num; i++) {
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

void particles_from_entity(PARTICLE_SYSTEM system, ENTITY ent, void(*callback)(PARTICLE)) {
	if(system && ent->obj) {
		u32 count = (ent->obj->texture_pixels > system->particle_count) ? 
			system->particle_count : ent->obj->texture_pixels;

		u32 j = 0;
		for(u32 i = 0; i < count; i++) {
			PARTICLE p = &system->particles[i];
			p->x = ent->x+ent->obj->texture_data[j];
			p->y = ent->y+ent->obj->texture_data[j+1];
			memmove(p->rgb, ent->obj->texture_data+j+2, 3);
			p->dead = 0;
			callback(p);

			j += 5;
		}

	}
}


void init_engine(char* title, u16 width, u16 height, int flags) {
	
	window = NULL;
	g_st = NULL;

	min_access_index = 0;
	max_access_index = 0;
	g_program = 0;

	g_use_rgb[0] = 0.0;
	g_use_rgb[1] = 0.0;
	g_use_rgb[2] = 0.0;

	if(!(g_st = malloc(sizeof *g_st))) {
		fprintf(stderr, "Failed to allocate memory for g_state_t!\n");
		return;
	}

	g_st->flags = flags;
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
	glfwWindowHint(GLFW_DECORATED, (flags & FLG_INIT_WINDOW_BORDER));
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
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
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
	//glfwSwapInterval(1);
	glfwGetWindowSize(window, &g_st->window_width, &g_st->window_height);

	if(g_st->window_width*g_st->window_height <= 0) {
		fprintf(stderr, "glfwGetWindowSize returned width:%i, height:%i\n", g_st->window_width, g_st->window_height);
		shutdown_engine();
		return;
	}

	g_st->max_col = g_st->window_width/PIXEL_SIZE;
	g_st->max_row = g_st->window_height/PIXEL_SIZE;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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
	g_st->render_mode = RENDER_MODE_UPDATE_FRAME;
}

void start_engine(void(*callback)(STATE), void(*start_callback)(STATE)) {
	
	// create default shader
	const char* fragment_source = 
		"#version 330\n"
		"in vec3 f_col;"

		"void main() {"
			"vec3 col = f_col;"
			"gl_FragColor = vec4(col, 1.0);"
		"}";

	const char* vertex_source =
		"#version 330\n"
		"layout(location = 0) in vec2 i_pos;"
		"layout(location = 1) in vec3 i_col;"
		
		"out vec3 f_col;"

		"void main() {"
			"gl_Position = vec4(i_pos.x, i_pos.y, 0.0, 1.0);"
			"f_col = i_col;"
		"}"
		;

	g_program = create_shader(vertex_source, fragment_source);

	glPointSize(PIXEL_SIZE);
	glUseProgram(g_program);

	int frames = 0;
	double second_counter = 0.0;

	if(start_callback != NULL) {
		start_callback(g_st);
	}


	glBindBuffer(GL_ARRAY_BUFFER, g_st->vbo);
	glfwWaitEventsTimeout(1.0);

	while(!glfwWindowShouldClose(window)) {
		g_st->time = glfwGetTime();

		glClear(GL_COLOR_BUFFER_BIT);
	
		g_st->buffer = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	
		if(g_st->buffer != NULL) {
			memset(g_st->buffer, -2, g_st->buffer_length);

			glUseProgram(g_program);
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
		if(g_st->render_mode == RENDER_MODE_WAIT_EVENTS) {
			glfwWaitEvents();
		}
		else {
			glfwPollEvents();
		}
	}
	
	glDeleteProgram(g_program);
}

void shutdown_engine() {
	if(g_st != NULL) {
		glDeleteVertexArrays(1, &g_st->vao);
		glDeleteBuffers(1, &g_st->vbo);
		
		free(g_st->grid);
		free(g_st);
	}
	
	if(window != NULL) {
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

	const float rw = w/g_st->window_width;
	const float rh = h/g_st->window_height;



	glUseProgram(0);
	glColor3f(g_use_rgb[0], g_use_rgb[1], g_use_rgb[2]);

	glBegin(GL_QUADS);
	glVertex2f(rx, ry);
	glVertex2f(rx+rw, ry);
	glVertex2f(rx+rw, ry-rh);
	glVertex2f(rx, ry-rh);

	glEnd();
	glUseProgram(g_program);
}

void draw_area(float x, float y, float w, float h) {
	_draw_f(x, y, w*PIXEL_SIZE*2, h*PIXEL_SIZE*2);
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
	if(obj != NULL && obj->texture_data != NULL) {
		if(obj->flags & OBJ_FLG_LOADED && obj->flags & OBJ_FLG_VISIBLE) {
			u32 p = 0;
			if(obj->blink > 0.0) {
				obj->counter += g_st->dt;
				if(obj->counter > obj->blink_rate) {
					if(obj->counter > obj->blink_rate*2.0) {
						obj->counter = 0.0;
					}
					return;
				}
				obj->blink -= g_st->dt;
			}

			if(obj->shake > 0.0) {
				x += randomi(-obj->shake_x,obj->shake_x);
				y += randomi(-obj->shake_y,obj->shake_y);
				obj->shake -= g_st->dt;
			}

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
}

void draw_line(u16 x0, u16 y0, u16 x1, u16 y1) {
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

void rotate(float x, float y, float* x_out, float* y_out, float angle) {
	if(x_out) {
		*x_out = x*cos(angle)-y*sin(angle);
	}
	if(y_out) {
		*y_out = x*sin(angle)+y*cos(angle);
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

void refresh_font_values(FONT f) {
	f->char_width = f->size*(f->glyph_width+f->column_space);
	f->char_height = f->size*(f->glyph_height+f->row_space);
}

FONT create_psf2_font(char* path) {
	FONT f = NULL;
	gzFile file = NULL;

	file = gzopen(path, "r");
	if(!file) {
		fprintf(stderr, "Failed to open font file \"%s\", %p\n", path, path);
		perror("gzopen");
		goto giveup;
	}

	if((f = malloc(sizeof *f)) == NULL) {
		goto giveup;
	}

	f->data = NULL;
	const u64 size = gzfread(f, PSF2_HEADER_SIZE, 1, file); 
	
	if(size == 0) {
		free(f);
		goto giveup;
	}
	
	if(gzeof(file)) {
		perror("gzeof");
		goto giveup;
	}

	if(
			f->magic[0] != 0x72 ||
			f->magic[1] != 0xb5 ||
			f->magic[2] != 0x4a ||
			f->magic[3] != 0x86) {
		fprintf(stderr, "Wrong magic bytes! \"%s\"\n", path);
		free(f);
		goto giveup;
	}

	f->data_size = f->num_glyphs*f->glyph_height;

	if((f->data = malloc(f->data_size)) == NULL) {
		free(f);
		goto giveup;
	}
		
	gzfread(f->data, f->data_size, 1, file);

	f->size = 0.5;
	f->row_space = 2;
	f->column_space = 2;
	f->tab_width = 4;
	f->char_width = 0;
	f->char_height = 0;

	refresh_font_values(f);

giveup:
	if(file) {
		gzclose(file);
	}

	return f;
}

void destroy_font(FONT f) {
	if(f != NULL) {
		if(f->data != NULL) {
			free(f->data);
		}
		free(f);
	}
}

void draw_char(FONT f, char c, float x, float y) {
	float ox = x;
	for(u16 i = 0; i < f->glyph_height; i++) {
		int g = f->data[c*f->glyph_height+i];
		for(int j = 0; j < 8; j++) {
			if(g & 0x80) {
				draw_area(x, y, f->size, f->size);
			}

			g = g << 1;
			x += f->size;
		}
		y += f->size;
		x = ox;
	}
}

void draw_text(FONT f, char* txt, u64 size, float x, float y) {
	u64 i = 0;
	float ox = x;

	while(1) {
		const char c = *txt;
		if(c == 0) {
			break;
		}

		draw_char(f, c, x, y);
		x += f->char_width;

		i++;
		if(size > 0 && i > size) { break; }
		txt++;
	}

}




