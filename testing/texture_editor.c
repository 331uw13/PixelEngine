#include "../src/pixel_engine.h"
#include "../src/utils.h"

#include <unistd.h>
#include <stdio.h>
#include <math.h>

#define IS_KEYDOWN(key) glfwGetKey(engine_win(), key) == GLFW_PRESS



#define GRID_PIXEL 8
static float draw_grid_x = 0.0;
static float draw_grid_y = 0.0;
static float draw_grid_s = 0.0;

static u8 r_value = 0;
static u8 g_value = 0;
static u8 b_value = 0;

static u8 prev_r_value = 0;
static u8 prev_g_value = 0;
static u8 prev_b_value = 0;


double mx = 0.0;
double my = 0.0;

struct color_t {
	char x;
	char y;
	char r;
	char g;
	char b;
	
	u8 e;
};

#define MOUSE_DOWN (1<<0)
#define MOUSE_PREV_DOWN (1<<2)

#define GRID_SIZE 256
#define COLOR_PALETTE_SIZE 8
struct color_t grid[GRID_SIZE];
static FONT font = NULL;
static int flags = 0;

static int selected_color = 0;
static struct color_t saved_colors[COLOR_PALETTE_SIZE*3];

static char* texture_filename = NULL;
static int texture_filename_size = 0;


void pick_from_palette(u8 i) {
	if(i < COLOR_PALETTE_SIZE) {
		struct color_t* c = &saved_colors[i];
		r_value = c->r;
		g_value = c->g;
		b_value = c->b;
	}
}


void write_texture() {
	
	FILE* f = fopen(texture_filename, "wb");

	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			struct color_t* c = &grid[x+16*y];
			c->x = x;
			c->y = y;
			if(c->e) {
				fwrite(c, 1, 5, f);
			}
			c->x *= 8;
			c->y *= 8;
		}
	}

	fclose(f);
}

void open_texture() {

	FILE* f = fopen(texture_filename, "rb");
	struct color_t* c = malloc(sizeof *c);

	u32 num_saved_colors = 0;

	while(1) {
		
		fread(c, 1, 5, f);
		const u32 index = c->x+16*c->y;

		if(index < GRID_SIZE) {
			struct color_t* ptr = &grid[index];
			ptr->e = 1;
			ptr->x = c->x*8;
			ptr->y = c->y*8;
			ptr->r = c->r;
			ptr->g = c->g;
			ptr->b = c->b;

			if(num_saved_colors < COLOR_PALETTE_SIZE) {
				int save = 1;
				for(int i = 0; i < num_saved_colors; i++) {
					struct color_t* col = &saved_colors[i];
					if(col->r == ptr->r && col->g == ptr->g && col->b == ptr->b) {
						save = 0;
						break;
					}
				}

				if(save) {
					saved_colors[num_saved_colors].r = ptr->r;
					saved_colors[num_saved_colors].g = ptr->g;
					saved_colors[num_saved_colors].b = ptr->b;
					num_saved_colors++;
				}

			}

		}

		if(feof(f)) {
			break;
		}
	}

	free(c);
	fclose(f);
}

void draw_preview(double time) {
	
	float sx = 100;
	float sy = GRID_PIXEL*16+15;


	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			struct color_t* c = &grid[x+16*y];
			if(!c->e) { continue; }
			use_color(c->r, c->g, c->b);
			draw_pixel(sx+x, sy+y);
		}
	}
}

void delete_grid() {
	for(int i = 0; i < GRID_SIZE; i++) {
		grid[i].e = 0;
	}
}


void move_grid(int x_off, int y_off) {

	struct color_t grid_2[GRID_SIZE];
	memset(grid_2, 0, sizeof grid_2);

	int abort = 0;

	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			struct color_t* c = &grid[x+16*y];
			if(!c->e) { continue; }
			int rx = x+x_off;
			int ry = y+y_off;

			if((rx < 0 || ry < 0) || (rx >= 16 || ry >= 16)) {
				abort = 1;
				break;
			}

			int index = rx+16*ry;
			if(index > GRID_SIZE) { 
		   		abort = 1;
				break;
			}


			memmove(&grid_2[index], c, sizeof *c);
			grid_2[index].x = rx*8;
			grid_2[index].y = ry*8;

		}
		if(abort) { break; }
	}

	if(!abort) {
		memmove(grid, grid_2, sizeof *grid*GRID_SIZE);
	}
}

void flip_grid_x() {
	struct color_t grid_2[GRID_SIZE];
	memset(grid_2, 0, sizeof grid_2);
	
	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			struct color_t* c = &grid[x+16*y];
			if(!c->e) { continue; }
			int rx = 15-x;
			int index = rx+16*y;

			memmove(&grid_2[index], c, sizeof* c);
			grid_2[index].x = rx*8;
		}
	}
		
	memmove(grid, grid_2, sizeof *grid*GRID_SIZE);
}


void key_callback(GLFWwindow* win, int key, int sc, int act, int mods) {
	if(act == GLFW_PRESS) {
		switch(key) {

			case GLFW_KEY_UP:
				move_grid(0, -1);
				break;
			
			case GLFW_KEY_DOWN:
				move_grid(0, 1);
				break;
			
			case GLFW_KEY_LEFT:
				move_grid(-1, 0);
				break;
			
			case GLFW_KEY_RIGHT:
				move_grid(1, 0);
				break;

			case GLFW_KEY_DELETE:
				delete_grid();
				break;

			case GLFW_KEY_1:
				if(selected_color > 0) { 
					selected_color--;
				}
				break;
			
			case GLFW_KEY_2:
				if(selected_color < COLOR_PALETTE_SIZE-1) { 
					selected_color++;
				}
				break;

			case GLFW_KEY_3:
				pick_from_palette(selected_color);
				break;

			case GLFW_KEY_BACKSPACE:
				if(texture_filename_size > 0) {
					texture_filename_size--;
					texture_filename[texture_filename_size] = 0;
				}
				break;

			default:break;
		}
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if(action == GLFW_PRESS) {
		flags |= MOUSE_DOWN;
	}
	else {
		flags &= ~MOUSE_DOWN;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}

void handle_player_hold_input(STATE st) {
	if(IS_KEYDOWN(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(engine_win(), 1);
	}

}

static int focus_for_input = 0;

void slider_u8(int x, int y, u8* ptr, u8 r, u8 g, u8 b) {	
	use_color(3, 3, 3);

	float w = 13*PIXEL_SIZE;
	float h = 2*PIXEL_SIZE;
	draw_area(x, y, w, h);

	if(IS_KEYDOWN(GLFW_KEY_SPACE) && inside_rect(x, y, w, h, mx, my)) {
		focus_for_input = 0;
		*ptr = (u8)map(mx-x, 0, 25*2, 0, 16);
		if(*ptr > 16) {
			*ptr = 16;
		}
	}

	use_color(r, g, b);
	float vx = map(*ptr, 0, 16, 0, 25*2-1);
	draw_area(x+vx, y, PIXEL_SIZE, h);
}

void button(FONT font, int x, int y, char* text, u64 len, void(*callback)()) {
	if(callback == NULL) { return; }
	int wi = (len+1)*font->char_width+4;
	draw_area(x, y, wi, font->char_height+3);

	int on = (mx >= x && my >= y) && 
		     (mx <= x+wi+2 && my <= y+font->char_height+3);
	
	use_color(1, on ? 15 : 9, 3);
	if(text != NULL) {
		draw_text(font, text, len, x+2, y+2);
	}

	if(on && (flags & MOUSE_DOWN)) {
		callback();
	}
}

void save_selected_color() {
	prev_r_value = r_value;
	prev_g_value = g_value;
	prev_b_value = b_value;
	saved_colors[selected_color].r = r_value;
	saved_colors[selected_color].g = g_value;
	saved_colors[selected_color].b = b_value;
}


static double blink_counter = 0.0;
static int cursor_blink = 0;

void set_focus_for_text_input() {
	focus_for_input = 1;
}

void draw_filename_thing(STATE st) {
	use_color(3,3,3);
	float name_y = draw_grid_y+GRID_PIXEL*16+10;

	button(font, 5, name_y, NULL, 15, set_focus_for_text_input);
	
	use_color(10, 10, 10);
	float fs = font->size;
	if(texture_filename_size > 0) {
		draw_text(font, texture_filename, texture_filename_size, 5, name_y);
	}

	if(!focus_for_input) { return; }

	blink_counter += st->dt;
	if(blink_counter > 0.2){
		cursor_blink = !cursor_blink;
		blink_counter = 0.0;
	}

	if(cursor_blink) {
		use_color(8, 8, 8);
		draw_area(texture_filename_size*font->char_width+5, name_y, 3, font->char_height-1);
	}


}

void char_callback(GLFWwindow* window, u32 codepoint) {
	if(focus_for_input && texture_filename_size < 16) {
		texture_filename[texture_filename_size] = codepoint;
		texture_filename_size++;
	}
}

void illuminate_color() {
	r_value = CLAMP_VALUE(r_value+1, 0, 16);
	g_value = CLAMP_VALUE(g_value+1, 0, 16);
	b_value = CLAMP_VALUE(b_value+1, 0, 16);
}

void decrease_color() {
	r_value = CLAMP_VALUE(r_value-1, 0, 16);
	g_value = CLAMP_VALUE(g_value-1, 0, 16);
	b_value = CLAMP_VALUE(b_value-1, 0, 16);
}


void update(STATE st) {
	handle_player_hold_input(st);
	glfwGetCursorPos(engine_win(), &mx, &my);

	mx /= PIXEL_SIZE;
	my /= PIXEL_SIZE;

	float sldr_x = draw_grid_x+draw_grid_s/2+5;
	slider_u8(sldr_x, draw_grid_y, &r_value, 13, 8, 8);
	slider_u8(sldr_x, draw_grid_y+10, &g_value, 8, 13, 8);
	slider_u8(sldr_x, draw_grid_y+20, &b_value, 8, 8, 13);


	use_color(r_value, g_value, b_value);
	draw_area(sldr_x, draw_grid_y+30, 8, 8);
	
	use_color(prev_r_value, prev_g_value, prev_b_value);
	draw_area(sldr_x+10, draw_grid_y+30, 8, 8);

	use_color(3, 3, 3);
	button(font, sldr_x+1, draw_grid_y+50, "Save Color", 9, save_selected_color);
	
	use_color(3, 3, 3);
	button(font, sldr_x+1, draw_grid_y+80, "Write", 4, write_texture);
	
	use_color(3, 3, 3);
	button(font, sldr_x+25, draw_grid_y+30, "+", 1, illuminate_color);
	
	use_color(3, 3, 3);
	button(font, sldr_x+40, draw_grid_y+30, "-", 1, decrease_color);
	
	use_color(3, 3, 3);
	button(font, sldr_x+1, draw_grid_y+100, "Flip X", 6, flip_grid_x);

	draw_filename_thing(st);

	int row = 0;
	int col = 0;
	for(int i = 0; i < COLOR_PALETTE_SIZE; i++) {
		struct color_t* c = &saved_colors[i];
		use_color(c->r, c->g, c->b);
		int x = sldr_x+col*7;
		int y = draw_grid_y+70+row;
		draw_area(x, y, 5, 5);
		col++;
		if(i == 7) {
			row += 10;
			col = 0;
		}

		if(i == selected_color) {
			use_color(8, 10, 8);
			draw_area(x, y+6, 5, 1);
			
		}

	}
	
	
	

	use_color(2, 2, 2);
	for(int i = 0; i < 17; i++) {
		int inc = i*(GRID_PIXEL);

		_draw_f(draw_grid_x+inc, draw_grid_y, 2.0, draw_grid_s*PIXEL_SIZE);
		_draw_f(draw_grid_x, draw_grid_y+inc, draw_grid_s*PIXEL_SIZE, 2.0);
	
	}

	for(int y = 0; y < GRID_SIZE; y++) {
		struct color_t* c = &grid[y];
		if(c->e) {
			use_color(c->r, c->g, c->b);
			draw_area(draw_grid_x+c->x+0.2, draw_grid_y+c->y+0.2, GRID_PIXEL-0.4, GRID_PIXEL-0.4);
		}
	}

	draw_preview(st->time);

	if(inside_rect(draw_grid_x, draw_grid_y, GRID_PIXEL*16-1.5, GRID_PIXEL*16-1.5, mx, my)) {
		float gx0 = mx-GRID_PIXEL;
		float gy0 = my-GRID_PIXEL;

		focus_for_input = 0;

		st->flags |= FLG_HIDE_CURSOR;
		
		float gx = round(gx0/GRID_PIXEL-0.25)*GRID_PIXEL;
		float gy = round(gy0/GRID_PIXEL-0.25)*GRID_PIXEL;

		if(gx < 0) {
			gx = 0.0;
		}

		if(gy < 0) {
			gy = 0;
		}

		int rx = map(gx, 0.0, GRID_PIXEL*16, 0, 16);
		int ry = map(gy, 0.0, GRID_PIXEL*16, 0, 16);

		int indx = rx+16*ry;
		if(indx > GRID_SIZE) { indx = GRID_SIZE; }
		struct color_t* c = &grid[indx];
		

		if(c) {
			if(c->e) {
				use_color(16-c->r, 16-c->g*0.5, 16-c->b);
			}
			else {
				use_color(7, 16, 7);
			}

			float x = draw_grid_x+gx;
			float y = draw_grid_y+gy;

			draw_area(x, y, 2, 2);
			
			dim_color(4);
			draw_area(mx, y, 1.2, 1.2);
			draw_area(x, my, 1.2, 1.2);
			
			draw_area(x, y, mx-x, 0.3);
			draw_area(x, y, 0.3, my-y);
			
			//draw_area(x, y, 0.5, 2);

			/*
			draw_area(x+GRID_PIXEL-1.5, y, 2, 0.5);
			draw_area(x+GRID_PIXEL, y, 0.5, 2);
			
			draw_area(x, y+GRID_PIXEL, 2, 0.5);
			draw_area(x, y+GRID_PIXEL-1.5, 0.5, 2);
			draw_area(x+GRID_PIXEL-1.5, y+GRID_PIXEL, 2, 0.5);
			draw_area(x+GRID_PIXEL, y+GRID_PIXEL-1.5, 0.5, 2);
			*/
			
			if(IS_KEYDOWN(GLFW_KEY_SPACE)) {
				c->e = 1;
				c->r = r_value;
				c->g = g_value;
				c->b = b_value;
				c->x = gx;
				c->y = gy;
			}
			else if(IS_KEYDOWN(GLFW_KEY_E)) {
				c->e = 0;
			}
			else if(IS_KEYDOWN(GLFW_KEY_C)) {
				if(c->e) {
					r_value = c->r;
					g_value = c->g;
					b_value = c->b;
				}
			}

		}
	}
	else {
		st->flags &= ~FLG_HIDE_CURSOR;
		st->mouse_x = mx;
		st->mouse_y = my;
		use_color(16, 16, 10);
		draw_pixel(mx, my);
		draw_pixel(mx+1, my);
		draw_pixel(mx, my+1);
	}	
	flags &= ~MOUSE_DOWN;
}


void engine_ready(STATE st) {
	if(st) {
		back_color(1, 1, 1);
		flags = 0;
		selected_color = 0;
		draw_grid_x = 5.0;
		draw_grid_y = 5.0;
		draw_grid_s = (GRID_PIXEL*2)*16;

		blink_counter = 0.0;
		cursor_blink = 0;
		focus_for_input = 0;

		if(texture_filename_size <= 0) {
			for(int i = 0; i < COLOR_PALETTE_SIZE; i++) {
				saved_colors[i].r = randomi(0, 16);
				saved_colors[i].g = randomi(0, 16);
				saved_colors[i].b = randomi(0, 16);
			}	
		
			prev_r_value = 0;
			prev_g_value = 0;
			prev_b_value = 0;

			r_value = randomi(0, 16);
			g_value = randomi(0, 16);
			b_value = randomi(0, 16);
		}
	}
}

int main(int argc, char** argv) {
	delete_grid();

	texture_filename = malloc(16);
	texture_filename_size = 0;

	if(argc > 1) {
		texture_filename_size = strlen(argv[1]);
		memmove(texture_filename, argv[1], texture_filename_size);
		if(access(texture_filename, R_OK) != -1) {
			open_texture();
		}
	}


	init_engine("idk something", 800, 800, 0);

	font = create_psf2_font("Topaz.psf.gz");
	font->size = 0.5;

	refresh_font_values(font);

	glfwSetCharCallback(engine_win(), char_callback);
	glfwSetKeyCallback(engine_win(), key_callback);
	glfwSetMouseButtonCallback(engine_win(), mouse_button_callback);
	glfwSetScrollCallback(engine_win(), scroll_callback);
	start_engine(update, engine_ready);

	destroy_font(font);
	shutdown_engine();
	return 0;
}

