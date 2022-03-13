#ifndef ENEMY_H
#define ENEMY_H

#include "weapon.h"

#define ENEMY_DEFAULT 0



struct enemy_t {
	int x;
	int y;
	int health;
	int max_health;

	struct weapon_t weapon;
};


void spawn_enemy(int y, int max_health, struct weapon_t* weapon);


#endif
