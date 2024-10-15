#ifndef MODEL_MGR_H
#define MODEL_MGR_H

#include <stdlib.h>
#include "model.h"

struct model_mgr {
	const char **paths;
	struct model **models;
	size_t models_num;
};

struct model *model_mgr_get(struct model_mgr *mgr, const char *path);
void model_mgr_free(struct model_mgr *mgr);

#endif
