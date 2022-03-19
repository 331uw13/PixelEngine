#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "../src/pixel_engine.h"
#include "../src/utils.h"
#include "enemy.h"
#include "weapon.h"


#define IS_KEYDOWN(key) glfwGetKey(engine_win(), key) == GLFW_PRESS


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

#define PLAYER_MAX_HEALTH 100
#define MAX_AUDIO_FILES   16
#define NUM_CHANNELS      4
#define MAX_REGION        5

struct player_t {
	int x;
	int y;
	int health;
	u8 y_region;

	struct weapon_t w;

	struct object_t ship;
	struct object_t cursor;
	
};

static u32 loaded_audio_files;
static struct player_t plr;

static struct particle_system_t ship_prtcl;
static struct particle_system_t star_prtcl;
static struct particle_system_t proj_prtcl;

static int max_column;
static int max_row;

static Mix_Chunk* g_audio[MAX_AUDIO_FILES];

#define DEFAULT_ZONE  20

#define XSPEED 2.0
#define YSPEED 2.0


int move_to_region(int reg) {
	return reg*(max_row/MAX_REGION);
}


void draw_grid(struct g_state_t* st) {

	use_color(5, 5, 8);
	for(int y = 0; y < st->max_row-1; y++) {
		for(int x = 0; x < st->max_col-1; x++) {
		
			if(st->grid[grid_index(x, y)]) {
				draw_pixel(x, y, 1);
			}
		}
	
	}
}

void player_shoot() {
	struct particle_t* p = &proj_prtcl.particles[proj_prtcl.last_dead];
	if(p != NULL && p->dead) {
		const float t = glfwGetTime();
		if(weapon_shoot(&plr.w)) {
			p->dead = 0;
			p->x = plr.x+10;
			p->y = plr.y+3;
			
			Mix_HaltChannel(0);
			Mix_PlayChannel(0, g_audio[0], 0);
		}
	}
}


void key_callback(GLFWwindow* win, int key, int sc, int act, int mods) {
	if(act != GLFW_RELEASE) {
		switch(key) {
			
			case GLFW_KEY_ENTER:
				player_shoot();
				break;

			case GLFW_KEY_S:
				if(plr.y_region < MAX_REGION) {
					plr.y_region++;
					plr.y = move_to_region(plr.y_region);
				}
				break;

			case GLFW_KEY_W:
				if(plr.y_region > 0) {
					plr.y_region--;
					plr.y = move_to_region(plr.y_region);
				}
				break;

			default:break;
		}
	}
}

void handle_player_input(struct g_state_t* st) {

	if(IS_KEYDOWN(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(engine_win(), 1);
	}
/*
	if(IS_KEYDOWN(GLFW_KEY_W)) {
		plr.y -= (IS_KEYDOWN(GLFW_KEY_LEFT_CONTROL)) ? YSPEED*2 : YSPEED;
	}
	if(IS_KEYDOWN(GLFW_KEY_S)) {
		plr.y += (IS_KEYDOWN(GLFW_KEY_LEFT_CONTROL)) ? YSPEED*2 : YSPEED;
	}
*/	
	if(IS_KEYDOWN(GLFW_KEY_A)) {
		plr.x -= XSPEED;
	}
	
	if(IS_KEYDOWN(GLFW_KEY_D)) {
		plr.x += XSPEED;
	}

	if(IS_KEYDOWN(GLFW_KEY_X)) {
		draw_grid(st);
	}


	if(plr.x < 3.0) {
		plr.x = 3;
	}
	else if(plr.x+11 > max_column) {
		plr.x = max_column-11;
	}
	if(plr.y+6 >= max_row) {
		plr.y = max_row-6;
	}
	if(plr.y <= 1) {
		plr.y = 1;
	}

	plr.ship.x = plr.x;
	plr.ship.y = plr.y;
}

void proj_particle_update(struct particle_t* p, struct g_state_t* st) {
	if(!p->dead) {
		p->x += 5;
		if(p->x >= max_column) {
			p->dead = 1;
			proj_prtcl.last_dead = p->index;
		}


		use_color(5, 12, 3);
		draw_area(p->x-7, p->y, 5, 1, 1);

		use_color(2, 16, 2);
	}
	else {
		proj_prtcl.last_dead = p->index;
	}
}

void ship_particle_update(struct particle_t* p, struct g_state_t* st) {

	if(p->dead) {
		if(!IS_KEYDOWN(GLFW_KEY_A)) {
			p->x = plr.x;
			p->y = plr.y+randomi(0, 5);

			p->max_lifetime = randomf(0.3, 1.5);
			p->ay = randomf(0.0, 0.5)-0.25;
			p->vx = randomf(3.0, 4.0);

			p->rgb[0] = 0;
			p->rgb[1] = 0;
			p->rgb[3] = 0;
		}
	}

	p->x -= p->vx;
	p->y -= p->ay;

	float f = lerp(p->lifetime/(p->max_lifetime+0.3), 0.0, p->max_lifetime);
	p->rgb[0] = 16-16*f;
	p->rgb[2] = 16*f;

	use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
}

void star_particle_update(struct particle_t* p, struct g_state_t* st) {
	p->x -= p->vx;
	use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
	
	if(p->x <= 1) {
		p->lifetime = 0.0;
		p->vx = randomf(2.0, 10.0);

		const int ll = lerp(p->vx/10.0, 0.0, 8.0);

		int i = randomi(0, 3);

		p->rgb[0] = ll+i;
		p->rgb[1] = ll+i;
		p->rgb[2] = ll+randomi(2, 5);

		p->x = max_column-1;
		p->y = randomi(1, max_row-2);


	}
}


void update(struct g_state_t* st) {
	update_particles(&star_prtcl);

	max_column = st->max_col-2;
	max_row    = st->max_row;

/*
	int lx = max_column/2;
	int ly = max_row/2;



	st->lights[0].x = lx;
	st->lights[1].y = ly;

	update_light(0);
*/

	handle_player_input(st);
	update_particles(&ship_prtcl);
	update_particles(&proj_prtcl);

	draw_object(&plr.ship, 1);
}

#define SHIP_OUTLINE 5, 5, 5
#define SHIP_WINDOW  1, 10, 16
#define SHIP_WING    8, 8, 8
#define ENEMY_WING   9, 9, 9


int main() {

	int r_code = 0;

	if(Mix_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "failed to initialize SDL for audio\n%s\n", SDL_GetError());
		r_code = errno;
		goto main_end;
	}

	if(!Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 512) < 0) {
		fprintf(stderr, "failed to open audio device\n%s\n", SDL_GetError());
		SDL_Quit();
		r_code = errno;
		goto main_end;
	}

	if(!Mix_AllocateChannels(NUM_CHANNELS)) {
		fprintf(stderr, "failed to allocate channels\n%s\n", SDL_GetError());
		SDL_Quit();
		r_code = errno;
		goto main_end;
	}


	loaded_audio_files = 0;

	g_audio[0] = Mix_LoadWAV("test.wav");

	plr.x = DEFAULT_ZONE;
	plr.y_region = MAX_REGION/2;
	plr.y = move_to_region(plr.y_region);
	plr.health = PLAYER_MAX_HEALTH;

	char cursor_data[] = {
		0,  1,  5, 12, 5,
		0, -1,  5, 12, 5,
		1,  0,  5, 16, 5,
	   -1,  0,  5, 16, 5,
	};

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


	load_object_mem(&plr.cursor, cursor_data, sizeof(cursor_data));
	load_object_mem(&plr.ship, ship_data, sizeof(ship_data));
	
	init_engine("something");

	create_particle_system(32, 1, proj_particle_update, &proj_prtcl);
	create_particle_system(32, 1, ship_particle_update, &ship_prtcl);
	create_particle_system(16, 0, star_particle_update, &star_prtcl);
	
	for(int i = 0; i < MAX_WEAPON_ABILITIES; i++) {
		plr.w.ability[i] = AB_DEFAULT;
	}

	weapon_update_stats(&plr.w);

	glfwSetKeyCallback(engine_win(), key_callback);

	start_engine(update);
	shutdown_engine();

	unload_object(&plr.cursor);
	unload_object(&plr.ship);

	for(int i = 0; i < loaded_audio_files; i++) {
		Mix_FreeChunk(g_audio[i]);
	}
	Mix_CloseAudio();
	Mix_Quit();

main_end:
	return r_code;
}








