#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "weapon.h"
#include "utils.h"


WEAPON create_weapon() {
	WEAPON w = malloc(sizeof *w);
	if(w != NULL) {
		w->usable = 1;
		w->rgb[0] = 16;
		w->rgb[1] = 16;
		w->rgb[2] = 16;
		w->damage = 50;
		w->projectile_count = 1;
		w->projectile_speed = 320;
		w->critical = 30;
		w->max_energy = 100;
		w->energy_regen_rate = 0.8;
		w->energy = w->max_energy;
		w->shoot_rate = 0.25;
		w->last_shot_time = 0.0;
		w->dt = 0.0;
		w->data = NULL;
	}

	return w;
}

void destroy_weapon(WEAPON w) {
	if(w) {
		w->usable = 0;
		free(w);
	}
}

void weapon_update(WEAPON w) {
	if(w && w->usable) {
		w->dt = glfwGetTime()-w->last_shot_time;
		if(w->dt <= 0.0) {
			return;
		}

		if(w->energy < w->max_energy) {
			if(w->dt > w->shoot_rate*2.0) {
				w->energy += w->dt*w->energy_regen_rate;
				if(w->energy > w->max_energy) {
					w->energy = w->max_energy;
				}
			}
		}
	}
}

u8 weapon_can_shoot(WEAPON w) {
	return (w && w->usable && (w->dt > w->shoot_rate));
}

u16 weapon_get_damage(WEAPON w) {
	u16 d = w->damage;

	const u8 do_critical = (randomi(0, 100) <= w->critical);
	if(do_critical) {
		d += randomi(50, 100);
	}



	return d;
}

u16 weapon_shoot(WEAPON w) {
	u16 d = 0;
	if(weapon_can_shoot(w)) {
		w->last_shot_time = glfwGetTime();
		d = weapon_get_damage(w);
		w->energy -= 1.0/w->dt;
		if(w->energy < 0.0) {
			w->energy = 0.0;
		}
	}

	return d;
}

