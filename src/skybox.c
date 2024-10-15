#include <stdio.h>
#include "glad/gl.h"
#include "stb_image.h"
#include "shader.h"
#include "skybox.h"

float vertices[] = {
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f 
};

unsigned int indices[] = {
    4, 5, 6,  4, 6, 7,
    0, 1, 2,  0, 2, 3,
    3, 2, 6,  3, 6, 7,
    0, 1, 5,  0, 5, 4,
    1, 2, 6,  1, 6, 5,
    0, 3, 7,  0, 7, 4 
};

void skybox_load(struct skybox *box, const char *faces[6]) {
	glGenVertexArrays(1, &box->vao);
	glGenBuffers(1, &box->vbo);
	glGenBuffers(1, &box->ebo);

	glBindVertexArray(box->vao);

	glBindBuffer(GL_ARRAY_BUFFER, box->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glGenTextures(1, &box->tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, box->tex);

	int x, y, comp_num;
	for (int i = 0; i < 6; i++) {
		stbi_uc *data = stbi_load(faces[i], &x, &y, &comp_num, 4);
		if (data == NULL) {
			fprintf(stderr, "Failed loading skybox");
			abort();
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void skybox_draw(struct skybox *box, GLuint shader) {
	glBindVertexArray(box->vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, box->tex);

	shader_set_int(shader, "skybox", 0);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void skybox_free(struct skybox *box) {
	glDeleteVertexArrays(1, &box->vao);
	glDeleteBuffers(1, &box->vbo);
	glDeleteBuffers(1, &box->ebo);
	glDeleteTextures(1, &box->tex);
}
