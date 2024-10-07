#ifndef ARRLIST_H
#define ARRLIST_H

#include "memutils.h"

#define arrlist(type) struct { type *data; size_t len; size_t cap; }

#define arrlist_init(arr, c) do { \
	arr.len = 0; \
	arr.cap = c > 0 ? c : 16; \
	arr.data = mallocs(arr.cap * sizeof(*arr.data)); \
} while(0)

#define arrlist_push(arr, item) do { \
	if (arr.len == arr.cap) { \
		arr.cap *= 2; \
		arr.data = reallocs(arr.data, arr.cap * sizeof(*arr.data)); \
	} \
	arr.data[arr.len++] = item; \
} while(0)

#define arrlist_free(arr) do { \
	free(arr.data); \
	arr.len = 0; \
	arr.cap = 0; \
	arr.data = NULL; \
} while(0)

#endif
