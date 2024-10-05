#ifndef MODEL_FMT_H
#define MODEL_FMT_H

#include <stdint.h>

#pragma pack(1)

struct header_fmt {
	uint32_t meshes_off;
	uint32_t meshes_num;

	uint32_t indices_off;
	uint32_t indices_num;

	uint32_t attrs_off;
	uint32_t attrs_num;

	uint32_t textures_off;
	uint32_t textures_num;

	uint32_t materials_off;
	uint32_t materials_num;
};

struct mesh_fmt {
	int16_t material_idx;
	uint32_t indices_idx;
	uint32_t indices_num;
};

struct attr_fmt {
	float position[3];
	float normal[3];
	float uv[2];
};

struct texture_fmt {
	uint32_t off;
	uint32_t len;
	uint16_t width;
	uint16_t height;
};

struct material_fmt {
	int16_t albedo_map;
	float albedo_tint[4];
};

#pragma pack()

#endif
