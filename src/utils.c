#include "utils.h"

static int g_seed = 0;


float normalize(float t, float min, float max) {
	return (t-min)/(max-min);
}

float lerp(float t, float min, float max) {
	return (max-min)*t+min;
}

float map(float t, float s_min, float s_max, float d_min, float d_max) {
	return lerp(normalize(t, s_min, s_max), d_min, d_max);
}

void set_seed(int s) {
	g_seed = s;
}

int random_gen() {
	g_seed = 0x343FD*g_seed+0x269EC3;
	return (g_seed >> 16)&RANDOM_GEN_MAX;
}

int randomi(int min, int max) {
	return random_gen()%(max-min)+min;
}

float randomf(float min, float max) {
	return min+((float)random_gen())/((float)RANDOM_GEN_MAX/(max-min));
}

