#ifndef MODEL_FMT_H
#define MODEL_FMT_H

#include <stdint.h>

#pragma pack(1)

struct header_fmt {
	uint32_t meshes_off;
	uint32_t meshes_num;

	uint32_t indices_off;
	uint32_t indices_num;

	uint32_t vertices_off;
	uint32_t vertices_num;

	uint32_t textures_off;
	uint32_t textures_num;

	uint32_t materials_off;
	uint32_t materials_num;
};

struct mesh_fmt {
	uint16_t material_idx;
	uint32_t indices_idx;
	uint32_t indices_num;
};

struct texture_fmt {
	uint32_t off;
	uint32_t len;
	uint16_t width;
	uint16_t height;
};

struct material_fmt {
	uint16_t diffuse_map_idx;
	uint16_t specular_map_idx;
};

#pragma pack()

#endif
