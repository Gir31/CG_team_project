#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

typedef struct {
	char name[256];   // 재질 이름
	float Ka[3];      // Ambient color (R, G, B)
	float Kd[3];      // Diffuse color (R, G, B)
	float Ks[3];      // Specular color (R, G, B)
	float Ns;         // Specular exponent
	float d;          // Transparency (1.0 = opaque)
} Material;


typedef struct {
	float x, y, z; // Vertex coordinates
} Vertex;

typedef struct {
	float u, v; // Texture coordinates
} TextureCoord;

typedef struct {
	float x, y, z; // Normal vector
} Normal;

typedef struct {
	unsigned int v1, v2, v3, v4;         // Vertex indices
	unsigned int vt1, vt2, vt3, vt4;    // Texture indices
	unsigned int vn1, vn2, vn3, vn4;    // Normal indices
} Face;

typedef struct {
	Vertex* vertices;           // Vertex positions
	TextureCoord* texture_coords; // Texture coordinates
	Normal* normals;            // Normals
	Face* faces;                // Faces
	Material material;

	size_t vertex_count;        // Number of vertices
	size_t texture_count;       // Number of texture coordinates
	size_t normal_count;        // Number of normals
	size_t face_count;          // Number of faces
} Object;

void read_newline(char* str) {
	char* pos;
	if ((pos = strchr(str, '\n')) != NULL)
		*pos = '\0';
}

void read_mtl_file(const char* filename, Material** materials, size_t* material_count) {

	FILE* file;
	fopen_s(&file, filename, "r");
	if (!file) {
		perror("Error opening MTL file");
		exit(EXIT_FAILURE);
	}

	char line[MAX_LINE_LENGTH];
	Material* material_list = NULL;
	size_t count = 0;

	Material current_material = { 0 };

	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (strncmp(line, "newmtl", 6) == 0) {
			if (current_material.name[0] != '\0') {
				material_list = (Material*)realloc(material_list, (count + 1) * sizeof(Material));
				material_list[count++] = current_material;
				memset(&current_material, 0, sizeof(Material)); // 새 재질 초기화
			}
			sscanf_s(line + 7, "%255s", current_material.name, (unsigned)_countof(current_material.name));
		}
		else if (strncmp(line, "Ka", 2) == 0) {
			sscanf_s(line + 2, "%f %f %f", &current_material.Ka[0], &current_material.Ka[1], &current_material.Ka[2]);
		}
		else if (strncmp(line, "Kd", 2) == 0) {
			sscanf_s(line + 2, "%f %f %f", &current_material.Kd[0], &current_material.Kd[1], &current_material.Kd[2]);
		}
		else if (strncmp(line, "Ks", 2) == 0) {
			sscanf_s(line + 2, "%f %f %f", &current_material.Ks[0], &current_material.Ks[1], &current_material.Ks[2]);
		}
		else if (strncmp(line, "Ns", 2) == 0) {
			sscanf_s(line + 2, "%f", &current_material.Ns);
		}
		else if (strncmp(line, "d", 1) == 0) {
			sscanf_s(line + 1, "%f", &current_material.d);
		}
	}

	if (current_material.name[0] != '\0') {
		material_list = (Material*)realloc(material_list, (count + 1) * sizeof(Material));
		material_list[count++] = current_material;
	}

	fclose(file);

	*materials = material_list;
	*material_count = count;
}

void read_obj_file_with_mtl(const char* filename, Object* model) {
	FILE* file;
	fopen_s(&file, filename, "r");
	if (!file) {
		perror("Error opening OBJ file");
		exit(EXIT_FAILURE);
	}

	char line[MAX_LINE_LENGTH];
	model->vertex_count = 0;
	model->texture_count = 0;
	model->normal_count = 0;
	model->face_count = 0;

	Material* materials = NULL;
	size_t material_count = 0;
	char current_material[256] = { 0 };
	char mtl_filename[256] = { 0 };

	// 첫 번째 스캔: 데이터 크기 계산 및 MTL 파일 이름 읽기
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (strncmp(line, "mtllib", 6) == 0) {
			sscanf_s(line + 7, "%255s", mtl_filename, (unsigned)_countof(mtl_filename));
		}
		else if (line[0] == 'v' && line[1] == ' ') {
			model->vertex_count++;
		}
		else if (line[0] == 'v' && line[1] == 't') {
			model->texture_count++;
		}
		else if (line[0] == 'v' && line[1] == 'n') {
			model->normal_count++;
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			model->face_count++;
		}
	}

	// MTL 파일 읽기
	if (mtl_filename[0] != '\0') {
		char mtl_path[MAX_LINE_LENGTH];
		snprintf(mtl_path, sizeof(mtl_path), "%s", mtl_filename);
		read_mtl_file(mtl_path, &materials, &material_count);
	}

	// 메모리 할당
	model->vertices = (Vertex*)malloc(model->vertex_count * sizeof(Vertex));
	model->texture_coords = (TextureCoord*)malloc(model->texture_count * sizeof(TextureCoord));
	model->normals = (Normal*)malloc(model->normal_count * sizeof(Normal));
	model->faces = (Face*)malloc(model->face_count * sizeof(Face));

	size_t vertex_index = 0, texture_index = 0, normal_index = 0, face_index = 0;

	// 두 번째 스캔: 데이터 읽기
	fseek(file, 0, SEEK_SET);
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (line[0] == 'v' && line[1] == ' ') {
			sscanf_s(line + 2, "%f %f %f",
				&model->vertices[vertex_index].x,
				&model->vertices[vertex_index].y,
				&model->vertices[vertex_index].z);
			vertex_index++;
		}
		else if (line[0] == 'v' && line[1] == 't') {
			sscanf_s(line + 2, "%f %f",
				&model->texture_coords[texture_index].u,
				&model->texture_coords[texture_index].v);
			texture_index++;
		}
		else if (line[0] == 'v' && line[1] == 'n') {
			sscanf_s(line + 2, "%f %f %f",
				&model->normals[normal_index].x,
				&model->normals[normal_index].y,
				&model->normals[normal_index].z);
			normal_index++;
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			unsigned int v[4], vt[4], vn[4];
			int matches = sscanf_s(line + 2,
				"%u/%u/%u %u/%u/%u %u/%u/%u %u/%u/%u",
				&v[0], &vt[0], &vn[0],
				&v[1], &vt[1], &vn[1],
				&v[2], &vt[2], &vn[2],
				&v[3], &vt[3], &vn[3]);
			if (matches == 12) {
				model->faces[face_index].v1 = v[0] - 1;
				model->faces[face_index].vt1 = vt[0] - 1;
				model->faces[face_index].vn1 = vn[0] - 1;

				model->faces[face_index].v2 = v[1] - 1;
				model->faces[face_index].vt2 = vt[1] - 1;
				model->faces[face_index].vn2 = vn[1] - 1;

				model->faces[face_index].v3 = v[2] - 1;
				model->faces[face_index].vt3 = vt[2] - 1;
				model->faces[face_index].vn3 = vn[2] - 1;

				model->faces[face_index].v4 = v[3] - 1;
				model->faces[face_index].vt4 = vt[3] - 1;
				model->faces[face_index].vn4 = vn[3] - 1;

				face_index++;
			}
		}
		else if (strncmp(line, "usemtl", 6) == 0) {
			sscanf_s(line + 7, "%255s", current_material, (unsigned)_countof(current_material));
			std::cout << current_material << std::endl;
		}
	}

	fclose(file);

	// 재질 정보 할당
	if (materials && material_count > 0) {
		for (size_t i = 0; i < material_count; ++i) {
			if (strcmp(materials[i].name, current_material) == 0) {
				model->material = materials[i];
				break;
			}
		}
	}

	// 재질 정보 해제
	if (materials) {
		free(materials);
	}
}




void read_obj_file(const char* filename, Object* model) {
	FILE* file;
	fopen_s(&file, filename, "r");
	if (!file) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	char line[MAX_LINE_LENGTH];
	model->vertex_count = 0;
	model->face_count = 0;
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (line[0] == 'v' && line[1] == ' ')
			model->vertex_count++;
		else if (line[0] == 'f' && line[1] == ' ')
			model->face_count++;
	}
	fseek(file, 0, SEEK_SET);
	model->vertices = (Vertex*)malloc(model->vertex_count * sizeof(Vertex));
	model->faces = (Face*)malloc(model->face_count * sizeof(Face));
	size_t vertex_index = 0;    size_t face_index = 0;
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (line[0] == 'v' && line[1] == ' ') {
			int result = sscanf_s(line + 2, "%f %f %f", &model->vertices[vertex_index].x,
				&model->vertices[vertex_index].y,
				&model->vertices[vertex_index].z);
			vertex_index++;
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			unsigned int vertices[128]; // 최대 128개의 정점 처리 가능
			int vertex_count = 0;

			// Face 라인을 정점 배열로 파싱
			char* token = strtok(line + 2, " ");
			while (token != NULL && vertex_count < 128) {
				sscanf_s(token, "%u/%*u/%*u", &vertices[vertex_count]);
				vertices[vertex_count]--; // OBJ는 1부터 시작하므로 0부터 시작으로 변환
				vertex_count++;
				token = strtok(NULL, " ");
			}

			if (vertex_count < 3) {
				fprintf(stderr, "Error: Invalid face with less than 3 vertices\n");
				continue;
			}

			// 다각형을 삼각형으로 분할
			for (int i = 1; i < vertex_count - 1; i++) {
				if (face_index >= model->face_count) { // Face 배열 확장
					size_t new_size = model->face_count * 2;
					model->faces = (Face*)realloc(model->faces, new_size * sizeof(Face));
					if (!model->faces) {
						fprintf(stderr, "Failed to reallocate memory for faces\n");
						exit(EXIT_FAILURE);
					}
					model->face_count = new_size;
				}

				model->faces[face_index].v1 = vertices[0];
				model->faces[face_index].v2 = vertices[i];
				model->faces[face_index].v3 = vertices[i + 1];
				model->faces[face_index].v4 = 0; // 삼각형
				face_index++;
			}
		}


	}
	fclose(file);
}
