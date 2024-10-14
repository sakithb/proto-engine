#ifndef STATE_H
#define STATE_H

#include "camera.h"

struct state {
	int scr_width;
	int scr_height;

	float delta_time;
	float last_frame_time;

	struct camera camera;
};

#endif
