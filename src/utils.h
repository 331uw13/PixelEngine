#ifndef UTILS_H
#define UTILS_H

#define RANDOM_GEN_MAX 0x7FFF
#define CLAMP_VALUE(a, min, max) (a < min ? min : a > max ? max : a)


float normalize(float t, float min, float max);
float lerp(float t, float min, float max);
float map(float t, float s_min, float s_max, float d_min, float d_max);


int   random_gen();
int   randomi(int min, int max);
float randomf(float min, float max);
void  set_seed(int s);


#endif
