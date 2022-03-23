#include <GLFW/glfw3.h>
#include "weapon.h"
#include "../src/utils.h"



void weapon_update(struct weapon_t* w) {
	// TODO
}

void weapon_reload(struct weapon_t* w) {
	// TODO
}

int weapon_can_shoot(struct weapon_t* w) {
	int res = 0;
	if(w) {
		res = 
			(w->usable) && 
			(w->ammo >= w->projectile_count);
	}

	return res;
}

int weapon_shoot(struct weapon_t* w) {
	int damage = 0;

	if(weapon_can_shoot(w)) {
		const float t = glfwGetTime();
		if((t-w->last_shot_time) > w->shoot_rate) {
			w->last_shot_time = t;
			w->ammo -= w->projectile_count;
		
		
			damage = 1; // TODO...
		}
	}
	return damage;
}


void weapon_update_stats(struct weapon_t* w) {
	if(w) {
		
		
		if(w->ability[0] == AB_NONE) {
			w->usable = 0;
			return;
		}

		w->usable = 1;

		w->damage = 100;
		w->energy = 100;
		w->max_energy = 100;
		w->ammo   = 50;
		w->max_ammo = 50;
		w->projectile_count = 1;
		w->projectile_speed = 4;
		w->hit_radius = 1;
		w->critical   = 10;
		w->shoot_rate   = 0.15; // ?
		w->reload_time  = 1.0; // ?
		w->rgb[0] = 2;
		w->rgb[1] = 16;
		w->rgb[2] = 2;
		w->last_shot_time = glfwGetTime();

		for(int i = 0; i < MAX_WEAPON_ABILITIES; i++) {
			
			switch(w->ability[i]) {

				case AB_FASTER_RELOAD:
					w->reload_time -= 0.2;
					w->max_ammo -= 10;
					break;

				case AB_FASTER_SHOOT_RATE:
					w->shoot_rate -= 0.2;
					w->max_ammo += 10;
					break;

				case AB_SEEK:
					w->damage -= 10;
					w->shoot_rate += 0.1;
					w->max_energy -= 10;
					break;

				case AB_LARGEER_HIT_RADIUS:
					w->hit_radius += 2;
					w->shoot_rate += 0.05;
					break;

				case AB_FASTER_PROJECTILES:
					w->projectile_speed += 2;
					break;

				case AB_CRITICAL_BOOST:
					w->critical += 8;
					break;

				case AB_MULTIPLY_PROJECTILES:
					w->projectile_count *= 2;
					break;

				case AB_ENERGY_BOOST:
					w->max_energy += 10;
					break;
				
				
				case AB_NONE:
					w->usable = 0;
					break;

				case AB_DEFAULT:
				default:
					break;
			}
		}

	}

}




