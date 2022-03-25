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

