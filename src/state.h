#ifndef PIXEL_ENGINE_STATE_H
#define PIXEL_ENGINE_STATE_H


#define FLG_INIT_FULLSCREEN 0x1
#define FLG_INIT_WINDOW_BORDER 0x4
#define FLG_HIDE_CURSOR 0x8

#define RENDER_MODE_WAIT_EVENTS 0x2
#define RENDER_MODE_UPDATE_FRAME 0x3


struct g_state_t {
	int      flags;
	int      render_mode;
	double   time;
	double   dt;    // frame delta time

	int      fps;
	int      max_fps;
	int      window_width;
	int      window_height;
	int      max_row;
	int      max_col;
	u32      num_pixels;

	double mouse_x;
	double mouse_y;

	u8* grid;
	u64 grid_length;

	float*   buffer;
	u64      buffer_length;

	u32      vbo;
	u32      vao;
};

#define STATE struct g_state_t*



#endif
