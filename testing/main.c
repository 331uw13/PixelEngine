#include "../src/pixel_engine.h"
#include "../src/utils.h"

#define IS_KEYDOWN(key) glfwGetKey(engine_win(), key) == GLFW_PRESS


ENTITY player = NULL;

#define NUM_PLAYER_OBJECTS 7
OBJECT player_objects[NUM_PLAYER_OBJECTS];
static double anim_counter = 0;
static int anim_index = 0;
static int anim_flip_x = 0;
static int anim_flip_y = 0;


void key_callback(GLFWwindow* win, int key, int sc, int act, int mods) {
	if(act == GLFW_PRESS) {
		switch(key) {

			case GLFW_KEY_D:
				anim_index = 3;
				break;
			
			case GLFW_KEY_A:
				anim_index = 3;
				break;

			default:break;
		}
	}
}

void handle_player_hold_input(STATE st) {
	if(IS_KEYDOWN(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(engine_win(), 1);
	}

	if(IS_KEYDOWN(GLFW_KEY_D)) {
		player->x += st->dt*70.0;
		anim_flip_x = 0;
	}
	else if(IS_KEYDOWN(GLFW_KEY_A)) {
		player->x -= st->dt*70.0;
		anim_flip_x = 1;
	}
	else {
		if(anim_index >= 3) {
			anim_index = 0;
		}
	}

}


static float bx = 50.0;
static float by = 50.0;

void update(STATE st) {
	handle_player_hold_input(st);

	anim_counter += st->dt;
	
	if(anim_index >= 3) {
		if(anim_counter > 0.2) {
			anim_index++;
			if(anim_index > 6) {
				anim_index = 3;
			}
			anim_counter = 0;
		}
	}
	else {
		if(anim_counter > 0.9) {
			anim_index++;
			if(anim_index > 2) {
				anim_index = 0;
			}
			anim_counter = 0;
		}
	}

	if(IS_KEYDOWN(GLFW_KEY_SPACE)) {
	 	bx = st->mouse_x;
		by = st->mouse_y;
	}

	draw_object(player_objects[anim_index], player->x, player->y, anim_flip_x, 0);

	use_color(16, 16, 10);
}

void engine_ready(STATE st) {
	if(st) {
		anim_counter = 0.0;
		anim_index = 0;
		anim_flip_x = 0;
		anim_flip_y = 0;
		back_color(0, 0, 0);
		player = create_entity();

		player->x = st->max_col/2;
		player->y = st->max_row/2;

		player_objects[0] = create_object_from_file("kitty_idle0");
		player_objects[1] = create_object_from_file("kitty_idle1");
		player_objects[2] = create_object_from_file("kitty_idle2");
		player_objects[3] = create_object_from_file("kitty_run0");
		player_objects[4] = create_object_from_file("kitty_run1");
		player_objects[5] = create_object_from_file("kitty_run2");
		player_objects[6] = create_object_from_file("kitty_run3");
		
	}
}


void free_memory() {
	for(int i = 0; i < NUM_PLAYER_OBJECTS; i++) {
		destroy_object(player_objects[i]);
	}

	destroy_entity(player);
}

int main() {
	init_engine("default", 800, 800, 0);

	glfwSetKeyCallback(engine_win(), key_callback);
	start_engine(update, engine_ready);

	free_memory();
	shutdown_engine();
	return 0;
}








