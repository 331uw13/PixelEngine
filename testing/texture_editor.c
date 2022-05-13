#include "../src/pixel_engine.h"
#include "../src/utils.h"

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

double mx = 0.0;
double my = 0.0;

struct color_t {
	int x;
	int y;
	int r;
	int g;
	int b;
	
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


void write_texture() {
	
	FILE* f = fopen("texture", "wb");

	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			struct color_t* c = &grid[x+16*y];
			c->x = x;
			c->y = y;
			if(c->e) {
				fwrite(c, sizeof(int), 5, f);
			}
			c->x *= 8;
			c->y *= 8;
		}
	}

	fclose(f);
}

void open_texture(char* file) {

	FILE* f = fopen(file, "rb");
	struct color_t* c = malloc(sizeof *c);

	while(1) {
		
		fread(c, sizeof(int), 5, f);
		const u32 index = c->x+16*c->y;

		if(index < GRID_SIZE) {
			struct color_t* ptr = &grid[index];
			ptr->e = 1;
			ptr->x = c->x*8;
			ptr->y = c->y*8;
			ptr->r = c->r;
			ptr->g = c->g;
			ptr->b = c->b;

		}

		if(feof(f)) {
			break;
		}
	}

	free(c);
	fclose(f);
}

void draw_preview(double time) {
	
	float sx = 15;
	float sy = GRID_PIXEL*16+15;


	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			struct color_t* c = &grid[x+16*y];
			if(c->e) {
				use_color(c->r, c->g, c->b);
				draw_pixel(sx+x, sy+y);

				float rx = x-8;
				float ry = y-8;

				float rot_x = 0;
				float rot_y = 0;
				
				rotate(rx, ry, &rot_x, &rot_y, time*2.5);


				draw_pixel((sx*2+30)+rot_x, sy+rot_y+8);
			}
		}
	}
}

void delete_grid() {
	for(int i = 0; i < GRID_SIZE; i++) {
		grid[i].e = 0;
	}
}

void key_callback(GLFWwindow* win, int key, int sc, int act, int mods) {
	if(act == GLFW_PRESS) {
		switch(key) {

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

void slider_u8(int x, int y, u8* ptr, u8 r, u8 g, u8 b) {	
	use_color(3, 3, 3);

	float w = 13*PIXEL_SIZE;
	float h = 2*PIXEL_SIZE;
	draw_area(x, y, w, h);

	if(IS_KEYDOWN(GLFW_KEY_SPACE) && inside_rect(x, y, w, h, mx, my)) {
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
	draw_text(font, text, len, x+2, y+2);

	if(on && (flags & MOUSE_DOWN)) {
		callback();
	}
}

void save_color() {
	saved_colors[selected_color].r = r_value;
	saved_colors[selected_color].g = g_value;
	saved_colors[selected_color].b = b_value;
	if(selected_color < 15) {
		selected_color++;
	}
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


	const float sldr_end = sldr_x+13*PIXEL_SIZE;

	use_color(6, 6, 6);
	draw_area(sldr_end, draw_grid_y+2, 1, 5);
	draw_area(sldr_end, draw_grid_y+12, 1, 5);
	draw_area(sldr_end, draw_grid_y+22, 1, 5);
	
	use_color(r_value, 0, 0);
	draw_area(sldr_end+1, draw_grid_y+4, 6,1);
	use_color(0, g_value, 0);
	draw_area(sldr_end+1, draw_grid_y+14, 6,1);
	use_color(0, 0, b_value);
	draw_area(sldr_end+1, draw_grid_y+24, 6,1);

	use_color(r_value, 0, 0);
	draw_area(sldr_end+7, draw_grid_y+4, 1, 10);
	
	use_color(r_value, g_value, 0);
	draw_area(sldr_end+7, draw_grid_y+14, 1, 10);
	
	use_color(r_value, g_value, b_value);
	draw_area(sldr_end+7, draw_grid_y+24, 1, 10);
	draw_area(sldr_x+16, draw_grid_y+34, 44, 1);	

	draw_area(sldr_x, draw_grid_y+31, 16, 16);
	draw_area(sldr_x+16, draw_grid_y+31+10, 16, 1);
	draw_area(sldr_x+32, draw_grid_y+31+10, 1, 11);
	
	draw_area(sldr_x, draw_grid_y+49, 32, 1);
	draw_area(sldr_x, draw_grid_y+49, 1, 14);
	
	draw_area(sldr_x, draw_grid_y+62, selected_color*7+2, 1);

	use_color(3, 3, 3);
	button(font, sldr_x+1, draw_grid_y+50, "Save Color", 9, save_color);
	
	use_color(3, 3, 3);
	button(font, sldr_x+1, draw_grid_y+80, "Write", 4, write_texture);

	
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
			
			use_color(r_value, g_value, b_value);
			draw_area(x+2, y-8, 1, 6);
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
				c->r = saved_colors[selected_color].r;
				c->g = saved_colors[selected_color].g;
				c->b = saved_colors[selected_color].b;
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


		for(int i = 0; i < COLOR_PALETTE_SIZE; i++) {
			saved_colors[i].r = randomi(0, 16);
			saved_colors[i].g = randomi(0, 16);
			saved_colors[i].b = randomi(0, 16);
		}

		r_value = randomi(0, 16);
		g_value = randomi(0, 16);
		b_value = randomi(0, 16);

	}
}

int main(int argc, char** argv) {
	delete_grid();

	if(argc > 1) {

		open_texture(argv[1]);

	}


	init_engine("idk something", 800, 800, 0);

	font = create_psf2_font("Topaz.psf.gz");
	font->size = 0.5;

	refresh_font_values(font);

	glfwSetKeyCallback(engine_win(), key_callback);
	glfwSetMouseButtonCallback(engine_win(), mouse_button_callback);
	glfwSetScrollCallback(engine_win(), scroll_callback);
	start_engine(update, engine_ready);

	destroy_font(font);
	shutdown_engine();
	return 0;
}

