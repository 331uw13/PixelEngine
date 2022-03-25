#ifndef PIXEL_ENGINE_ENTITY_H
#define PIXEL_ENGINE_ENTITY_H

#include "object.h"
#include "weapon.h"

// TODO: inventory thingy????


struct entity_t {
	u32    id;
	u16    health;
	u16    max_health;
	u8     dead;

	float  x;
	float  y;

	OBJECT obj;
	WEAPON weapon;

	void* data; // for user
};

#define ENTITY struct entity_t*

ENTITY create_entity();
void   destroy_entity(ENTITY ent);
void   entity_set_next_id(u32 id);




#endif
