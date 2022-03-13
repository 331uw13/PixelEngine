#ifndef WEAPON_H
#define WEAPON_H

#define MAX_WEAPONS   3
#define MAX_WEAPON_ABILITIES 4

#define AB_NONE               -2  // ignore all abilities
#define AB_DEFAULT            -1  // no special effects or abilities
#define AB_EXPLODE_AFTER_HIT   0  //
#define AB_FAST_RELOAD         1  //
#define AB_FAST_SHOOT_RATE     2  // weapon can be used more often
#define AB_HOLD                3  // player can hold fire button
#define AB_SEEK                4  // seek and destroy
#define AB_ENERGETIC           5  // goes through enemies
#define AB_LARGE_HIT_RADIUS    6  // projectiles are bigger
#define AB_FAST_PROJECTILES    7  // 
#define AB_CRITICAL_BOOST      8  // better chance for critical hit
#define AB_MORE_PROJECTILES    9  // shoots more projectiles at once


struct weapon_t {
	
	int damage;
	int ammo;
	int reload_time;
	int projectile_count;

	int ability[4];
	int rgb[3];

	float shoot_rate;
};


void update_weapon_stats(struct weapon_t* w);


#endif
