#include "camera.h"
#include "model_mgr.h"
#include "state.h"

struct state state = {
	.scr_width = 1280,
	.scr_height = 720,
	.delta_time = 0.0f,
	.last_frame_time = 0.0f,
	.camera = (struct camera){0},
	.model_mgr = (struct model_mgr){0}
};
