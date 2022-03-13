#include <math.h>
#include "../src/pixel_engine.h"


#include "items.h"
#include "crafting.h"
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

#define MAX_ENEMIES 8
#define PLAYER_MAX_HEALTH 100

struct player_t {
	int x;
	int y;
	int health;
	int kills;

	int selected_weapon;
	struct weapon_t weapon[MAX_WEAPONS];

	struct object_t ship;
	struct object_t cursor;
};

struct enemy_t {
	int x;
	int y;
	int health;
	int max_health;
	u8 dead;
	struct weapon_t weapon;
};



static struct player_t plr;

static struct particle_system_t ship_prtcl;
static struct particle_system_t star_prtcl;
static struct particle_system_t proj_prtcl;

static struct weapon_t  enemy_weapons[4];
static struct enemy_t  enemies[MAX_ENEMIES];
static int enemy_count;

static int max_column;
static int max_row;

#define DEFAULT_ZONE  20

#define ACC_MAX_X 10.0
#define ACC_MAX_Y 10.0

#define XSPEED 2.0
#define YSPEED 2.0

int enter_was_pressed = 0;


void update_grid_space(struct g_state_t* st, int x, int y, int w, int h, int b) {

	for(int ry = y; ry < y+h; ry++) {
		for(int rx = x; rx < x+w; rx++) {
			st->grid[grid_index(rx, ry)] = b;
		}
	}

}


void spawn_enemy(int max_health, struct weapon_t* weapon) {
	if(enemy_count < MAX_ENEMIES) {
		struct enemy_t* e = &enemies[enemy_count++];

		e->x = max_column-12;
		e->y = randomi(10, max_row-10);
		e->dead = 0;
		e->health = max_health;
		e->max_health = max_health;
	}
}

void update_enemies(struct g_state_t* st) {
	if(enemy_count > 0) {
		struct enemy_t* e = NULL;

		for(int i = 0; i < MAX_ENEMIES; i++) {
			e = &enemies[i];
			if(!e->dead) {




				use_color(9, 6, 7);
				draw_area(e->x, e->y, 8, 8, 1);
				

				const int move_amount = -1;

				update_grid_space(st, e->x+8, e->y, -move_amount, 8, 0);
				
				
				e->x += move_amount;

				if((e->x <= 0) || (e->health <= 0)) {
					e->dead = 1;
					update_grid_space(st, e->x+1, e->y, 8, 8, 0);
					if(enemy_count > 0) {
						enemy_count--;
					}
				}
				else {

					update_grid_space(st, e->x, e->y, 8, 8, i+1);

				}
			}
		}
	}
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

void key_callback(GLFWwindow* win, int key, int sc, int act, int mods) {
	if(act == GLFW_PRESS) {
		switch(key) {
			case GLFW_KEY_ENTER:
				enter_was_pressed = 1;
				break;


			// FOR TESTING:
			case GLFW_KEY_T:
				spawn_enemy(50, &enemy_weapons[0]);
				break;

			default:break;
		}
	}
	else if(act == GLFW_RELEASE) {
		enter_was_pressed = 0;
	}
}

void handle_player_input(struct g_state_t* st) {


	if(IS_KEYDOWN(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(engine_win(), 1);
	}

	if(st->flags & FLG_MOUSE_LDOWN) {
	}

	if(st->flags & FLG_MOUSE_RDOWN) {
	}

	if(IS_KEYDOWN(GLFW_KEY_W)) {
		plr.y -= (IS_KEYDOWN(GLFW_KEY_LEFT_CONTROL)) ? YSPEED*2 : YSPEED;
	}
	
	if(IS_KEYDOWN(GLFW_KEY_S)) {
		plr.y += (IS_KEYDOWN(GLFW_KEY_LEFT_CONTROL)) ? YSPEED*2 : YSPEED;
	}
	
	if(IS_KEYDOWN(GLFW_KEY_A)) {
		plr.x -= XSPEED;
	}
	
	if(IS_KEYDOWN(GLFW_KEY_D)) {
		plr.x += XSPEED;
	}
	else if(plr.x > DEFAULT_ZONE) {
		plr.x -= 1;
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

void ship_particle_update(struct particle_t* p, struct g_state_t* st) {
	p->x -= p->vx;
	p->y -= p->ay;



	float f = lerp(p->lifetime/(p->max_lifetime+0.3), 0.0, p->max_lifetime);
	p->rgb[0] = 16-16*f;
	p->rgb[2] = 16*f;

	use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
}

void ship_particle_death(struct particle_t* p) {
	if(!IS_KEYDOWN(GLFW_KEY_A)) {
		p->x = plr.x;
		p->y = plr.y+randomi(0, 4);

		p->max_lifetime = randomf(0.3, 1.5);
		p->ay = randomf(0.0, 0.5)-0.25;
		p->vx = randomf(3.0, 4.0);

		p->rgb[0] = 0;
		p->rgb[1] = 0;
		p->rgb[3] = 0;
	}
}

void star_particle_death(struct particle_t* p) {
	
	p->vx = randomf(2.0, 10.0);

	const int ll = lerp(p->vx/10.0, 0.0, 8.0);


	int i = randomi(0, 3);

	p->rgb[0] = ll+i;
	p->rgb[1] = ll+i;
	p->rgb[2] = ll+randomi(2, 5);

	p->x = max_column-1;
	p->y = randomi(1, max_row-2);
}


void star_particle_update(struct particle_t* p, struct g_state_t* st) {
	p->x -= p->vx;
	use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
	
	if(p->x <= 1) {
		p->lifetime = 0.0;
		star_particle_death(p);
	}
}


int released_proj = 0;

void proj_particle_update(struct particle_t* p, struct g_state_t* st) {
	if(p->dead) {
		if(enter_was_pressed && !released_proj) {
			p->dead = 0;
			p->x = plr.x+4;
			p->y = plr.y+3;
			released_proj = 1;
			enter_was_pressed = 0;
		}
	}
	else {


		int hit_x = 0;

		if(ray_cast(p->x, p->y, p->x+5, p->y, &hit_x, NULL)) {
			const int i = st->grid[grid_index(hit_x, p->y)];
			if(i && (i < MAX_ENEMIES)) {
				struct enemy_t* e = &enemies[i-1];

				e->health -= 25;

				printf("%i\n", e->health);


			}

			p->dead = 1;
		}
		else {

			p->x += 5;
			if(p->x >= max_column) {
				p->dead = 1;
			}
			else {

				use_color(0, 5, 0);
				draw_area(p->x-5, p->y, 5, 1, 1);
				use_color(0, 16, 0);
			}	
		}
	}
}

void proj_particle_death(struct particle_t* p) {
}


void update(struct g_state_t* st) {
	update_particles(&star_prtcl);

	max_column = st->max_col-2;
	max_row    = st->max_row;


	update_enemies(st);
	
	handle_player_input(st);
	update_particles(&ship_prtcl);
	update_particles(&proj_prtcl);

	released_proj = 0;
	draw_object(&plr.ship, 1);
}

#define SHIP_OUTLINE 5, 5, 5
#define SHIP_WINDOW  1, 10, 16
#define SHIP_WING    8, 8, 8


int main() {

	plr.x = 0;
	plr.y = 0;
	plr.health = 100;

	enter_was_pressed = 0;
	released_proj = 1;

	enemy_count = 0;

	char cursor_data[] = {
		0,  1,  5, 12, 5,
		0, -1,  5, 12, 5,
		1,  0,  5, 16, 5,
	   -1,  0,  5, 16, 5,
	};

	char ship_data[] = {
	
		// outline:
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


		// window so the bugc mota?:
		
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

		// other:

		2, 1,   SHIP_WING,
		2, 2,   SHIP_WING,
		
		3, 2,   SHIP_WING,
		4, 2,   SHIP_WING,
		5, 2,   SHIP_WING,
		6, 2,   SHIP_WING,

		// thruster things?
		-1, -1,   SHIP_WING,
		 0, -1,   SHIP_WING,
		 1, -1,   SHIP_WING,
		
		-1, 1,   SHIP_WING,
		-1, 2,   SHIP_WING,
		
		-1, 4,   SHIP_WING,
		 0, 4,   SHIP_WING,
		 1, 4,   SHIP_WING,

	};
	
	if(!load_object_mem(&plr.cursor, cursor_data, sizeof(cursor_data))) {
		fprintf(stderr, "load_object_mem failed\n");
	}
	
	if(!load_object_mem(&plr.ship, ship_data, sizeof(ship_data))) {
		fprintf(stderr, "load_object_mem failed\n");
	}

	init_engine("something");

	create_particle_system(40, ship_particle_update, ship_particle_death, &ship_prtcl);
	ship_prtcl.max_lifetime = 0.8;
	ship_prtcl.can_die = 1;
	
	create_particle_system(10, star_particle_update, star_particle_death, &star_prtcl);
	star_prtcl.max_lifetime = 0.5;
	star_prtcl.can_die = 0;
		
	create_particle_system(50, proj_particle_update, proj_particle_death, &proj_prtcl);
	proj_prtcl.max_lifetime = 1.0;
	proj_prtcl.can_die = 0;
	
	for(int i = 0; i < MAX_WEAPONS; i++) {
		for(int j = 0; j < MAX_WEAPON_ABILITIES; j++) {
			plr.weapon[i].ability[j] = AB_DEFAULT;
		}
		update_weapon_stats(&plr.weapon[i]);
	}


	enemy_weapons[0].damage = 10;
	enemy_weapons[0].ammo = -1;
	enemy_weapons[0].reload_time = -1;
	enemy_weapons[0].projectile_count = 1;
	enemy_weapons[0].shoot_rate = 0.1;
	enemy_weapons[0].ability[0] = AB_NONE;
	enemy_weapons[0].rgb[0] = 16;
	enemy_weapons[0].rgb[1] = 4;
	enemy_weapons[0].rgb[2] = 4;

	glfwSetKeyCallback(engine_win(), key_callback);

	start_engine(update);
	shutdown_engine();

	unload_object(&plr.cursor);
	unload_object(&plr.ship);

}








