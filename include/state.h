#ifndef STATE_H
#define STATE_H

#include "camera.h"
#include "model_mgr.h"

struct state {
	int scr_width;
	int scr_height;

	float delta_time;
	float last_frame_time;

	struct camera camera;
	struct model_mgr model_mgr;
};

#endif
