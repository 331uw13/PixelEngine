#ifndef WEAPON_H
#define WEAPON_H

#define MAX_DAMAGE 
#define MAX_WEAPON_ABILITIES 3

#define AB_NONE               -2  // the weapon is unavailable
#define AB_DEFAULT            -1  // no special effects or abilities

#define AB_FASTER_RELOAD         0
#define AB_FASTER_SHOOT_RATE     1
#define AB_SEEK                  2
#define AB_LARGEER_HIT_RADIUS    3
#define AB_FASTER_PROJECTILES    4
#define AB_CRITICAL_BOOST        5
#define AB_MULTIPLY_PROJECTILES  6
#define AB_ENERGY_BOOST          7


struct weapon_t {
	int damage;
	int energy;
	int max_energy;
	int ammo;
	int max_ammo;
	int projectile_count;
	int projectile_speed;
	int hit_radius; // may hit other enemies nearby
	// but damage to the other enemies is depend on damage_energy
	
	int critical; // N% chance

	int ability[MAX_WEAPON_ABILITIES];
	int rgb[3];

	float shoot_rate;
	float reload_time;
	
	// other:
	int usable;
	float last_shot_time;
	
};

int  weapon_can_shoot(struct weapon_t* w);
int  weapon_shoot(struct weapon_t* w); // returns damage, does nothing if it cant shoot
void weapon_update(struct weapon_t* w);
void weapon_reload(struct weapon_t* w);
void weapon_update_stats(struct weapon_t* w);


#endif
