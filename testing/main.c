#include "../src/pixel_engine.h"
#include "../src/utils.h"



// IDEAS:
//
// - perma death
// - player reaches points where they can upgrade weapons or craft better weapons
// - infinite ammo but player has to reload the weapon
// - weapon types and abilities can be combined
// - some enemies may drop special items which are used for special crafting
// - regions:
//   - different enemies
//   - different environment
//   - their own color themes
// - few enemies releases poison or explodes dealing damage to the player
// - items that changes player combat style and movement style can be crafted or found


#define IS_KEYDOWN(key) glfwGetKey(engine_win(), key) == GLFW_PRESS

#define NUM_VERTICAL_REGIONS 5
static int g_max_row = 0;
static int g_max_col = 0;

static PARTICLE_SYSTEM star_system = NULL;



int move_to_region(int reg) {
	return reg*(g_max_row/NUM_VERTICAL_REGIONS);
}

void key_callback(GLFWwindow* win, int key, int sc, int act, int mods) {
	if(act != GLFW_RELEASE) {
		switch(key) {
			
			case GLFW_KEY_ENTER:
				break;

			case GLFW_KEY_R:
				break;

			case GLFW_KEY_S:
				break;

			case GLFW_KEY_W:
				break;

			default:break;
		}
	}
}

void handle_player_input(STATE st) {

	if(IS_KEYDOWN(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(engine_win(), 1);
	}

	if(IS_KEYDOWN(GLFW_KEY_A)) {
	}
	
	if(IS_KEYDOWN(GLFW_KEY_D)) {
	}

}


void star_particle_update(PARTICLE p, STATE st) {
	p->x -= p->vx*st->dt;
	use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
	
	if(p->x <= 1) {
		p->lifetime = 0.0;
		p->vx = randomf(200.0, 500.0);

		const int ll = lerp(randomf(0.0, 1.0), 0.0, 8.0);

		int i = randomi(0, 6);

		p->rgb[0] = ll+i;
		p->rgb[1] = ll+i;
		p->rgb[2] = ll+randomi(2, 5);

		p->x = g_max_col-1;
		p->y = randomi(1, g_max_row-2);

		use_color(16, 16, 16);
	}
}


void update(STATE st) {
	handle_player_input(st);
	update_particles(star_system);
	
	
}

void engine_ready(STATE st) {
	if(st) {
		g_max_row = st->max_row;
		g_max_col = st->max_col;
	}
}


#define SHIP_OUTLINE 5, 5, 5
#define SHIP_WINDOW  1, 10, 16
#define SHIP_WING    8, 8, 8
#define ENEMY_WING   9, 9, 9


int main() {

	char ship_data[] = {
		0, 0,   SHIP_OUTLINE,
		1, 0,   SHIP_OUTLINE,
		2, 0,   SHIP_OUTLINE,
		3, 0,   SHIP_OUTLINE,
		4, 0,   SHIP_OUTLINE,
		5, 0,   SHIP_OUTLINE,

		0, 1,   SHIP_OUTLINE,
		1, 1,   SHIP_OUTLINE,
		3, 1,   SHIP_OUTLINE,
		4, 1,   SHIP_OUTLINE,
		
		0, 2,   SHIP_OUTLINE,
		1, 2,   SHIP_OUTLINE,

		0, 3,   SHIP_OUTLINE,
		1, 3,   SHIP_OUTLINE,
		2, 3,   SHIP_OUTLINE,
		3, 3,   SHIP_OUTLINE,
		4, 3,   SHIP_OUTLINE,
		5, 3,   SHIP_OUTLINE,
		6, 3,   SHIP_OUTLINE,
		7, 3,   SHIP_OUTLINE,
		8, 3,   SHIP_OUTLINE,
		9, 3,   SHIP_OUTLINE,
		
		6, 0,   SHIP_WINDOW,
		7, 0,   SHIP_WINDOW,
		
		5, 1,   SHIP_WINDOW,
		6, 1,   SHIP_WINDOW,
		5, 1,   SHIP_WINDOW,
		7, 1,   SHIP_WINDOW,
		8, 1,   SHIP_WINDOW,

		7, 2,   SHIP_WINDOW,
		8, 2,   SHIP_WINDOW,
		9, 2,   SHIP_WINDOW,

		2, 1,   SHIP_WING,
		2, 2,   SHIP_WING,
		
		3, 2,   SHIP_WING,
		4, 2,   SHIP_WING,
		5, 2,   SHIP_WING,
		6, 2,   SHIP_WING,

		-1, -1,   SHIP_WING,
		 0, -1,   SHIP_WING,
		 1, -1,   SHIP_WING,
		
		-1, 1,   SHIP_WING,
		-1, 2,   SHIP_WING,
		
		-1, 4,   SHIP_WING,
		 0, 4,   SHIP_WING,
		 1, 4,   SHIP_WING,

	};

/*
	char enemy_default_data[] = {
		
		0, 0,  ENEMY_WING, 
		1, 0,  ENEMY_WING, 
		2, 0,  ENEMY_WING, 
		3, 0,  ENEMY_WING, 
		4, 0,  ENEMY_WING, 
		5, 0,  ENEMY_WING, 
		6, 0,  ENEMY_WING, 

		0, 7,  ENEMY_WING, 
		1, 7,  ENEMY_WING, 
		2, 7,  ENEMY_WING, 
		3, 7,  ENEMY_WING, 
		4, 7,  ENEMY_WING, 
		5, 7,  ENEMY_WING, 
		6, 7,  ENEMY_WING, 

		3, 1,  ENEMY_WING, 
		3, 2,  ENEMY_WING, 
		3, 6,  ENEMY_WING, 
		3, 5,  ENEMY_WING, 
		
		2, 3,  ENEMY_WING, 
		2, 4,  ENEMY_WING, 
		3, 3,  ENEMY_WING, 
		3, 4,  ENEMY_WING, 
		4, 3,  ENEMY_WING, 
		4, 4,  ENEMY_WING, 
		
		
		5, 2,  ENEMY_WING, 
		6, 2,  ENEMY_WING, 
		
		5, 5,  ENEMY_WING, 
		6, 5,  ENEMY_WING, 

	};
*/


	init_engine("something", 1200, 800, FLG_INIT_FULLSCREEN);
	
	star_system = create_particle_system(16, 0, star_particle_update);
	glfwSetKeyCallback(engine_win(), key_callback);


	start_engine(update, engine_ready);
	
	
	destroy_particle_system(star_system);
	shutdown_engine();



	return 0;
}








