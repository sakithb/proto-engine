#ifndef MEMUTILS_H
#define MEMUTILS_H

#include <stdio.h>
#include <stdlib.h>

#define mallocs(size) ({ \
		void *p = malloc(size); \
		if (p == NULL) { \
			perror("Failed to allocate (malloc) memory\n"); \
			abort(); \
		} \
		p; \
	})

#define callocs(nmemb, size) ({ \
		void *p = calloc(nmemb, size); \
		if (p == NULL) { \
			perror("Failed to allocate (calloc) memory\n"); \
			abort(); \
		} \
		p; \
	})

#endif
