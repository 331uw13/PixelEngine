#ifndef WEAPON_H
#define WEAPON_H

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


#endif
