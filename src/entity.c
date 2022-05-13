#include <stdlib.h>

#include "entity.h"

static u32 g_entity_counter = 0;


ENTITY create_entity() {
	ENTITY e = malloc(sizeof *e);

	if(e != NULL) {
		e->dead = 0;
		e->max_health = 100;
		e->health = e->max_health;

		e->obj = NULL;
		e->weapon = NULL;
		e->data = NULL;

		e->x = 0.0;
		e->y = 0.0;

		e->id = g_entity_counter;
		g_entity_counter++;

	}

	return e;
}

void destroy_entity(ENTITY ent) {
	if(ent) {
		free(ent);
	}
}

void entity_set_next_id(u32 id) {
	g_entity_counter = id;
}

void update_entity(ENTITY ent) {
	if(ent) {
		if(ent->health > ent->max_health) {
			ent->health = ent->max_health;
		}
		else if(ent->health <= 0) {
			ent->dead = 1;
		}

		if(ent->weapon != NULL) {
			weapon_update(ent->weapon);
		}
	}
}

void hurt_entity(ENTITY ent, u32 damage) {
	if(ent) {
		int h = ent->health-damage;
		if(h > 0) {
			ent->health = h;
		}
		else {
			ent->dead = 1;
			ent->health = 0;
		}
	}
}

void heal_entity(ENTITY ent, u32 amount) {
	if(ent) {
		int h = ent->health+amount;
		if(h < ent->max_health && h > 0) {
			ent->health = h;
		}
	}
}


