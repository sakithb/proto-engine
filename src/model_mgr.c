#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "memutils.h"
#include "model_mgr.h"

struct model *model_mgr_get(struct model_mgr *mgr, const char *path) {
	for (size_t i = 0; i < mgr->models_num; i++) {
		if (strcmp(mgr->paths[i], path) == 0) {
			return mgr->models[i];
		}
	}
	struct model *m = mallocs(sizeof(struct model));
	model_init(m, path);


	if (mgr->models_num == 0) {
		mgr->models_num++;
		mgr->models = mallocs(sizeof(struct model*));
		mgr->paths = mallocs(sizeof(char*));
	} else {
		mgr->models_num++;
		mgr->models = reallocs(mgr->models, mgr->models_num * sizeof(struct model*));
		mgr->paths = reallocs(mgr->paths, mgr->models_num * sizeof(char*));
	}

	mgr->models[mgr->models_num - 1] = m;
	mgr->paths[mgr->models_num - 1] = path;

	return m;
}

void model_mgr_free(struct model_mgr *mgr) {
	for (size_t i = 0; i < mgr->models_num; i++) {
		model_free(mgr->models[i]);
	}

	mgr->paths = NULL;
	mgr->models = NULL;
	mgr->models_num = 0;
}
