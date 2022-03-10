#include "../src/pixel_engine.h"
#include <math.h>

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


#define AB_EXPLODE_AFTER_HIT   0  //
#define AB_FAST_RELOAD         1  //
#define AB_FAST_SHOOT_RATE     2  // weapon can be used more often
#define AB_HOLD                3  // player can hold fire button
#define AB_SEEK                4  // seek and destroy
#define AB_ENERGETIC           5  // goes through enemies


struct weapon_t {
	
	int damage;
	int reload_time;
	int ammo;

	int ability[3];

};



struct player_t {
	int x;
	int y;
	int health;

	int selected_weapon;
	struct weapon_t weapon[3];

	float vx;
	float vy;
	float ax;
	float ay;

	struct object_t ship;
	struct object_t cursor;

};

static struct player_t plr;


static struct particle_system_t ship_prtcl;
static struct particle_system_t star_prtcl;

static int max_column;
static int max_row;

#define RAD        6.0
#define DEFAULT_ZONE  20

#define ACC_MAX_X 10.0
#define ACC_MAX_Y 10.0

#define XSPEED 2.0
#define YSPEED 2.0


void handle_player_input(struct g_state_t* st) {


	if(IS_KEYDOWN(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(engine_win(), 1);
	}

	if(st->flags & FLG_MOUSE_LDOWN) {
	}

	if(st->flags & FLG_MOUSE_RDOWN) {
	}


	plr.vx *= 0.1;
	plr.vy *= 0.1;

	if(IS_KEYDOWN(GLFW_KEY_W)) {
		plr.y -= YSPEED;
	}
	
	if(IS_KEYDOWN(GLFW_KEY_S)) {
		plr.y += YSPEED;
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



	if(plr.x < 3.0) {
		plr.x = 3;
		plr.vx = 0.0;
		plr.ax = 0.0;
	}
	else if(plr.x+11 > max_column) {
		plr.x = max_column-11;
		plr.vx = 0.0;
		plr.ax = 0.0;
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


void ship_particle_update(struct particle_t* p) {
	p->x -= randomf(5, 8);

	p->y -= p->ay;


	p->rgb[0] = 16-32*lerp(p->lifetime, 0.1, p->max_lifetime);
	p->rgb[2] = 8*lerp(p->lifetime, 0.1, p->max_lifetime);

	use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
}

void ship_particle_death(struct particle_t* p) {
	p->x = plr.x;
	p->y = plr.y+randomi(0, 4);

	p->max_lifetime = randomf(0.1, 0.8);
	p->ay = randomf(0.0, 1.0)-0.5;

	p->rgb[0] = 0;
	p->rgb[1] = 0;
	p->rgb[3] = 0;
}

void star_particle_death(struct particle_t* p) {
	
	p->vx = randomf(20.0, 25.0)+0.6;

	const int ll = lerp(normalize(p->vx, 21.0, 26.0), 20.0, 25.0);

	p->rgb[0] = ll+randomi(0, 3);
	p->rgb[1] = ll;
	p->rgb[2] = ll+randomi(2, 3);

	p->x = max_column-1;
	p->y = randomi(1, max_row-2);
}


void star_particle_update(struct particle_t* p) {
	p->x -= p->vx;
	use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
	
	if(p->x <= 1) {
		p->lifetime = 0.0;
		star_particle_death(p);
	}
}


void update(struct g_state_t* st) {

	max_column = st->max_col-2;
	max_row    = st->max_row;

	u16 mouse_x = 0;
	u16 mouse_y = 0;
	mouse_pos(&mouse_x, &mouse_y);
	
	handle_player_input(st);
	update_particles(&ship_prtcl);
	update_particles(&star_prtcl);

	draw_object(&plr.ship, 1);
}

#define SHIP_OUTLINE 5, 5, 5
#define SHIP_WINDOW  1, 10, 16
#define SHIP_WING    8, 8, 8


int main() {

	plr.x = 0;
	plr.y = 0;
	plr.health = 100;


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

	create_particle_system(30, ship_particle_update, ship_particle_death, &ship_prtcl);
	ship_prtcl.max_lifetime = 0.5;
	ship_prtcl.can_die = 1;
	
	create_particle_system(10, star_particle_update, star_particle_death, &star_prtcl);
	star_prtcl.max_lifetime = 0.5;
	star_prtcl.can_die = 0;

	start_engine(update);
	shutdown_engine();

	unload_object(&plr.cursor);
	unload_object(&plr.ship);

}
