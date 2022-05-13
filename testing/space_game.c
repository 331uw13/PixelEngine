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

#define NUM_VERTICAL_REGIONS 8
#define DEFAULT_ZONE 10
#define MAX_ENEMIES_VISIBLE 8

static int g_max_row = 0;
static int g_max_col = 0;

static PARTICLE_SYSTEM star_system = NULL;
static PARTICLE_SYSTEM proj_system = NULL;
static PARTICLE_SYSTEM hit_system  = NULL;

static ENTITY player = NULL;
static OBJECT enemy_objects[3];


struct g_wave_t {
	u32 wave_num;
	
	u16 num_enemies;
	u16 max_enemies;

	u8 num_enemies_visible;
	u8 in_progress;

	ENTITY enemies[MAX_ENEMIES_VISIBLE];
};

struct player_data_t {
	int y_region;
};

#define PLAYER_DATA_T struct player_data_t

static struct g_wave_t g_wave;

void _DEBUG_draw_grid(STATE st) {
	for(int y = 0; y < st->max_row; y++) {
		for(int x = 0; x < st->max_col; x++) {
			if(st->grid[grid_index(x, y)]) {
				use_color(5, 8, 16);
				draw_pixel(x, y);
			}
		}
	}
}


void update_grid_space(STATE st, u32 x, u32 y, u8 id) {
	st->grid[grid_index(x, y+3)] = id;
}

int move_to_region(int reg) {
	return reg*((g_max_row-5)/NUM_VERTICAL_REGIONS)+3;
}


void kill_enemy(ENTITY enemy) {
}

void spawn_enemy() {
	if(g_wave.num_enemies_visible < MAX_ENEMIES_VISIBLE) {
		for(int i = 0; i < MAX_ENEMIES_VISIBLE; i++) {
			ENTITY ent = g_wave.enemies[i];
			if(ent == NULL) {
				break;
			}

			if(ent->dead) {
				ent->id = i+1;
				ent->dead = 0;
				ent->health = 100;
				ent->x = g_max_col-10;
				ent->y = move_to_region(randomi(0, NUM_VERTICAL_REGIONS));
				g_wave.num_enemies_visible++;

				break;
			}
		}
	}
}

void update_wave(STATE st) {
	if(!g_wave.in_progress) {
	}

	for(int i = 0; i < MAX_ENEMIES_VISIBLE; i++) {
		ENTITY ent = g_wave.enemies[i];
		if(ent) {
			if(!ent->dead) {

				update_grid_space(st, ent->x, ent->y, 0);
				ent->x-=2;
				
				if(ent->x < 0) {
					ent->x = g_max_col;
				}

				update_grid_space(st, ent->x, ent->y, ent->id);
				draw_object(ent->obj, ent->x, ent->y);
				update_entity(ent);
			}
		}
	}
}



void player_shoot() {

	u16 damage = weapon_shoot(player->weapon);
	if(damage) {
		PARTICLE p = &proj_system->particles[proj_system->last_dead];

		p->dead = 0;
		p->x = player->x+10;
		p->y = player->y+3;

		p->u[0] = damage;
	}
}


void key_callback(GLFWwindow* win, int key, int sc, int act, int mods) {
	if(act != GLFW_RELEASE) {
		PLAYER_DATA_T* pd = (PLAYER_DATA_T*)player->data;
		if(pd == NULL) {
			return;
		}
		switch(key) {
			case GLFW_KEY_S:
				if(pd->y_region < NUM_VERTICAL_REGIONS) {
					pd->y_region++;
					player->y = move_to_region(pd->y_region);
				}
				break;

			case GLFW_KEY_W:
				if(pd->y_region > 0) {
					pd->y_region--;
					player->y = move_to_region(pd->y_region);
				}
				break;

			default:break;
		}
	}
}

#define PLAYER_X_SPEED 200.0

void handle_player_input(STATE st) {

	if(IS_KEYDOWN(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(engine_win(), 1);
	}

	if(IS_KEYDOWN(GLFW_KEY_A)) {
		player->x -= PLAYER_X_SPEED*st->dt;
		if(player->x < 5) {
			player->x = 5;
		}
	}
	
	if(IS_KEYDOWN(GLFW_KEY_D)) {
		player->x += PLAYER_X_SPEED*st->dt;
		const u32 m = g_max_col-15;
		if(player->x > m) {
			player->x = m;
		}
	}

	if(IS_KEYDOWN(GLFW_KEY_ENTER)) {
		player_shoot();
	}
}


void star_particle_update(PARTICLE p, STATE st) {
	p->x -= p->vx*st->dt;
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
	}
	
	use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
	draw_pixel(p->x, p->y);
}

void hit_particle_update(PARTICLE p, STATE st) {
	if(!p->dead) {

		p->x -= p->vx*st->dt;
		p->y += p->vy*st->dt;

		use_color(p->rgb[0], p->rgb[1], p->rgb[2]);
		draw_pixel(p->x, p->y);
	}
	
}

void spawn_hit_particles(int x, int y, u32 count) {
	
	int j = 0;
	
	for(int i = 0; i < hit_system->particle_count; i++) {
		PARTICLE p = &hit_system->particles[i];
		
		if(p != NULL) {
			if(p->dead) {
				p->dead = 0;
				p->max_lifetime = randomf(1.0, 1.5);
				p->lifetime = 0.0;
				p->x = x+randomi(-4, 4);
				p->y = y+randomi(-4, 4);


				float vx = 150.0;
				float vy = 8.0;

				p->vy = randomf(-vy, vy*2);
				p->vx = randomf(vx, vx-40.0);

				if(randomi(0, 2)) {
					p->rgb[0] = randomi(10, 16);
					p->rgb[1] = randomi(0, 2);
					p->rgb[2] = randomi(2, 5);
				}
				else {
					p->rgb[0] = randomi(13, 16);
					p->rgb[1] = randomi(5, 8);
					p->rgb[2] = randomi(2, 5);
				}


				j++;
				if(j >= count) {
					break;
				}
			}
		}

	}
}

void proj_particle_update(PARTICLE p, STATE st) {

	if(!p->dead) {

		int hit_x = 0;
		int hit_y = 0;
		int prev_x = p->x;
		p->x += player->weapon->projectile_speed*st->dt;

		if(ray_cast(prev_x-1, p->y, p->x+1, p->y, &hit_x, &hit_y)) {
			const u32 g_id = grid_index(hit_x, hit_y);
			u32 id = st->grid[g_id];
			if(id > 0 && id <= MAX_ENEMIES_VISIBLE) {
				id--;
				ENTITY enemy = g_wave.enemies[id];
				hurt_entity(enemy, p->u[0]);
				if(enemy->dead) {
					enemy->dead = 1;
					st->grid[g_id] = 0;
					g_wave.num_enemies_visible--;

					spawn_hit_particles(enemy->x, enemy->y, randomi(7, 30));
				}

				p->dead = 1;
				return;
			}
		}

		if(p->x > g_max_col-2) {
			p->dead = 1;
			proj_system->last_dead = p->index;
			return;
		}

		use_color(5, 16, 5);
		draw_pixel(p->x, p->y);
		draw_pixel(p->x-1, p->y);
		draw_pixel(p->x-2, p->y);

	}
	else {
		proj_system->last_dead = p->index;

	}
}


#define BAR_WIDTH 50
static float spawn_counter = 0.0;

void update(STATE st) {
	back_color(0, 0, 0);

	handle_player_input(st);
	update_particles(star_system);
	update_particles(proj_system);
	update_particles(hit_system);

	update_wave(st);
	update_entity(player);

	draw_object(player->obj, player->x, player->y);



	spawn_counter += st->dt;
	if(spawn_counter > 1.5) {
		spawn_counter = 0.0;
		spawn_enemy();
	}



	// draw status bars

	int health_x = map(player->health, 0, player->max_health, 0, BAR_WIDTH);

	use_color(16, 5, 5);
	for(int i = 0; i < BAR_WIDTH; i++) {
		if(i == health_x) {
			use_color(4, 4, 4);
		}
		draw_pixel(2+i, 2);
	}

	int energy_x = map(player->weapon->energy, 0, player->weapon->max_energy, 0, BAR_WIDTH);

	use_color(5, 15, 15);
	for(int i = 0; i < BAR_WIDTH; i++) {
		if(i == energy_x) {
			use_color(4, 4, 4);
		}
		draw_pixel(2+i, 4);
	}



	if(IS_KEYDOWN(GLFW_KEY_G)) {
		_DEBUG_draw_grid(st);
	}

}

void engine_ready(STATE st) {
	if(st) {
		spawn_counter = 0.0;
		g_max_row = st->max_row;
		g_max_col = st->max_col;

		player->x = DEFAULT_ZONE;
		player->y = move_to_region(NUM_VERTICAL_REGIONS/2);
	
		g_wave.num_enemies = 0;
		g_wave.max_enemies = 0;

		for(int i = 0; i < MAX_ENEMIES_VISIBLE; i++) {
			ENTITY ent = g_wave.enemies[i] = create_entity();
			ent->dead = 1;
			ent->obj = enemy_objects[0];
		}

		g_wave.num_enemies = 0;
		g_wave.max_enemies = 0;
		g_wave.num_enemies_visible = 0;
		g_wave.in_progress = 0;
		g_wave.wave_num = 0;

	}
}


#define SHIP_OUTLINE 5, 5, 5
#define SHIP_WINDOW  1, 10, 16
#define SHIP_WING    8, 8, 8
#define ENEMY_WING   9, 9, 9


int main() {

	char player_texture[] = {
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

	char enemy_default_texture[] = {
		
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


	init_engine("something", 1200, 800, FLG_INIT_FULLSCREEN);
	
	
	enemy_objects[0] = load_object_mem(enemy_default_texture, sizeof enemy_default_texture);
	

	hit_system  = create_particle_system(256, 1, hit_particle_update);
	proj_system = create_particle_system(32, 1, proj_particle_update);
	star_system = create_particle_system(16, 0, star_particle_update);
	
	player = create_entity();
	player->obj = load_object_mem(player_texture, sizeof player_texture);
	player->weapon = create_weapon();

	PLAYER_DATA_T* pd = malloc(sizeof *pd);
	pd->y_region = NUM_VERTICAL_REGIONS/2;

	player->data = (void*)pd;


	glfwSetKeyCallback(engine_win(), key_callback);
	start_engine(update, engine_ready);

	// unload particle systems
	destroy_particle_system(star_system);
	destroy_particle_system(proj_system);
	
	// unload player
	free(player->data);
	unload_object(player->obj);
	destroy_weapon(player->weapon);
	destroy_entity(player);


	// unload enemies
	for(int i = 0; i < MAX_ENEMIES_VISIBLE; i++) {
		g_wave.enemies[i] = create_entity();
	}

	// unload enemy objects
	unload_object(enemy_objects[0]);

	shutdown_engine();
	return 0;
}








