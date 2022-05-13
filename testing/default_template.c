#include "../src/pixel_engine.h"
#include "../src/utils.h"

#define IS_KEYDOWN(key) glfwGetKey(engine_win(), key) == GLFW_PRESS



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


void update(STATE st) {
	handle_player_hold_input(st);

	// every frame

}

void engine_ready(STATE st) {
	if(st) {
		back_color(0, 0, 2);

		puts("press ESC to exit.");
		// when engine is ready

	}
}

int main() {
	init_engine("default", 800, 500, 0);

	glfwSetKeyCallback(engine_win(), key_callback);
	start_engine(update, engine_ready);

	shutdown_engine();
	return 0;
}








