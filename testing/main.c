#include "../src/pixel_engine.h"

#include <math.h>
#include <time.h>

#define IS_KEYDOWN(key) glfwGetKey(engine_win(), key) == GLFW_PRESS
double mx = 0.0;
double my = 0.0;
int    mouse_down = 0;

FONT font = NULL;


void char_callback(GLFWwindow* win, u32 codepoint) {
	
}

void key_callback(GLFWwindow* win, int key, int sc, int act, int mods) {
	if(act == GLFW_PRESS) {
		switch(key) {

			default:break;
		}
	}
}

void handle_player_hold_input(STATE st) {
	if(IS_KEYDOWN(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(engine_win(), 1);
	}

}

// - TODO list
// - fun things when bored
// 	 - drawing flow line
// - time

static int g_flags = 0;
static char g_time_str[8];
static double g_counter = 0.0;
static float time_mover = 0.0;

#define FLG_UPDATE_ALL 0x1

void update_time() {
	time_t t = time(0);
	struct tm* tm = localtime(&t);

	time_mover = tm->tm_sec;

	snprintf(g_time_str, 6, "%02d:%02d", tm->tm_hour, tm->tm_min);
	g_time_str[7] = 0;
}

void update(STATE st) {
	handle_player_hold_input(st);
	glfwGetCursorPos(engine_win(), &mx, &my);
	mouse_down = glfwGetMouseButton(engine_win(), GLFW_MOUSE_BUTTON_LEFT);

	mx /= PIXEL_SIZE;
	my /= PIXEL_SIZE;

	/*
	g_counter += st->dt;
	if(g_counter > 1.0 || (g_flags & FLG_UPDATE_ALL)) {
		update_time();
		g_counter = 0;
	}

	use_color(16, 5, 8);
	draw_text(font, g_time_str, 5, 5, 5);

	use_color(3, 2, 3);
	draw_line(5, font->char_height+5, 65, font->char_height+5);
	
	use_color(16, 8, 10);
	draw_pixel(5+time_mover, font->char_height+5);
	*/



	use_color(16, 16, 10);
	draw_pixel(mx, my);
	draw_pixel(mx+1, my);
	draw_pixel(mx, my+1);


	g_flags &= ~FLG_UPDATE_ALL;
}

void engine_ready(STATE st) {
	if(st) {
		g_flags = 0;
		g_counter = 0.0;
		time_mover = 0.0;
		memset(g_time_str, 4, 0);
		back_color(0, 0, 0);

		g_flags |= FLG_UPDATE_ALL;
		
		st->render_mode = RENDER_MODE_WAIT_EVENTS;

	}
}

int main() {
	init_engine("info_thing", 800, 800, 0);
	font = create_psf2_font("Topaz.psf.gz");

	glfwSetKeyCallback(engine_win(), key_callback);
	glfwSetCharCallback(engine_win(), char_callback);
	start_engine(update, engine_ready);

	destroy_font(font);
	shutdown_engine();
	return 0;
}

