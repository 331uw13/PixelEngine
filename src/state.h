#ifndef PIXEL_ENGINE_STATE_H
#define PIXEL_ENGINE_STATE_H


#define FLG_INIT_FULLSCREEN (1<<0)
#define FLG_VBO_UPDATE      (1<<1)

#define MAX_LIGHTS 8

struct light_t {
	float brightness;
	float x;
	float y;
	float _reserved;
};


struct g_state_t {
	int      flags;
	double   time;
	double   dt;    // frame delta time
	u8       active_light_count;

	int      fps;
	int      max_fps;
	int      window_width;
	int      window_height;
	int      max_row;
	int      max_col;
	u32      num_pixels;

	u8* grid;
	u64 grid_length;

	struct light_t lights[MAX_LIGHTS];

	float*   buffer;
	u64      buffer_length;

	u32      vbo;
	u32      vao;
};

#define STATE           struct g_state_t*



#endif
