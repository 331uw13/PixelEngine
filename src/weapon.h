#ifndef PIXEL_ENGINE_WEAPON_H
#define PIXEL_ENGINE_WEAPON_H

#include "types.h"

// TODO (tomorrow):
//
//  - for each weapon there should be their own particle system.
//
//


struct weapon_t {
	u8  usable;
	u8  rgb[3]; // projectile color
	u16 damage;
	u16 projectile_count;
	u16 projectile_speed;
	u16 critical;
	
	float energy;
	float energy_regen_rate;
	float max_energy;

	float shoot_rate;
	float last_shot_time;
	float dt;  // time between previous and most recent use

	void* data; // for user
};

#define WEAPON struct weapon_t*

WEAPON create_weapon();
void   destroy_weapon(WEAPON w);

void weapon_update(WEAPON w);

u8   weapon_can_shoot(WEAPON w);
u16  weapon_get_damage(WEAPON w);
u16  weapon_shoot(WEAPON w); // returns damage


#endif
