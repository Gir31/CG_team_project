#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

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

typedef struct Model {
	Vertex* vertices;           // Vertex positions
	TextureCoord* texture_coords; // Texture coordinates
	Normal* normals;            // Normals
	Face* faces;                // Faces
	Material material;

	size_t vertex_count;        // Number of vertices
	size_t texture_count;       // Number of texture coordinates
	size_t normal_count;        // Number of normals
	size_t face_count;          // Number of faces

	struct Model* next;
} Model;

void read_newline(char* str) {
	char* pos;
	if ((pos = strchr(str, '\n')) != NULL)
		*pos = '\0';
}

void read_mtl_file(const char* filename, Material** materials, size_t* material_count) {
	std::cout << "=====[Read MTL File Start]==============================================================================================" << std::endl;

	FILE* file;
	fopen_s(&file, filename, "r");
	if (!file) {
		perror("Error opening MTL file");
		return;
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
			std::cout << "		New Material Add : [ " << current_material.name << " ]" << std::endl;
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

	std::cout << "=====[Read MTL File End]================================================================================================" << std::endl;
}

void match_normal_and_vertex(Model** model) {
	Model* new_model = *model;

	Normal* normal_list = (Normal*)malloc(new_model->normal_count * sizeof(Normal));

	for (int i = 0; i < new_model->normal_count; i++) {
		normal_list[i].x = new_model->normals[i].x;
		normal_list[i].y = new_model->normals[i].y;
		normal_list[i].z = new_model->normals[i].z;
	}
	for (int i = 0; i < new_model->face_count; i++) {
		new_model->normals[new_model->faces[i].v1].x = normal_list[new_model->faces[i].vn1].x;
		new_model->normals[new_model->faces[i].v1].y = normal_list[new_model->faces[i].vn1].y;
		new_model->normals[new_model->faces[i].v1].z = normal_list[new_model->faces[i].vn1].z;

		new_model->normals[new_model->faces[i].v2].x = normal_list[new_model->faces[i].vn2].x;
		new_model->normals[new_model->faces[i].v2].y = normal_list[new_model->faces[i].vn2].y;
		new_model->normals[new_model->faces[i].v2].z = normal_list[new_model->faces[i].vn2].z;

		new_model->normals[new_model->faces[i].v3].x = normal_list[new_model->faces[i].vn3].x;
		new_model->normals[new_model->faces[i].v3].y = normal_list[new_model->faces[i].vn3].y;
		new_model->normals[new_model->faces[i].v3].z = normal_list[new_model->faces[i].vn3].z;

		new_model->normals[new_model->faces[i].v4].x = normal_list[new_model->faces[i].vn4].x;
		new_model->normals[new_model->faces[i].v4].y = normal_list[new_model->faces[i].vn4].y;
		new_model->normals[new_model->faces[i].v4].z = normal_list[new_model->faces[i].vn4].z;
	}

	free(normal_list);
}

void prac_read_obj_file_with_mtl(const char* filename, Model** model) {
	std::cout << "=====[Read Obj File Start]==============================================================================================" << std::endl;\
	FILE* file;

	fopen_s(&file, filename, "r");
	if (!file) {
		perror("Error opening OBJ file");
		exit(EXIT_FAILURE);
	}

	char line[MAX_LINE_LENGTH];

	Material* materials = NULL; 
	size_t material_count = 0; 
	char current_material[256] = { 0 };
	char mtl_filename[256] = { 0 };

	Model* start_model = NULL;

	// 첫 번째 스캔: 데이터 크기 계산 및 MTL 파일 이름 읽기
	std::cout << "	[ 데이터 크기 계산 시작 ]" << std::endl;
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (strncmp(line, "mtllib", 6) == 0) {
			sscanf_s(line + 7, "%255s", mtl_filename, (unsigned)_countof(mtl_filename));
		}
		else if (line[0] == 'o' && line[1] == ' ') {
			if (start_model == NULL) { 
				start_model = (Model*)calloc(1, sizeof(Model));
				*model = start_model;
				std::cout << "		[ 시작 모델에 메모리 준비 성공 ]" << std::endl;
			}
			else {
				std::cout << "			[ 현재 모델에 메모리 할당 시작 ]" << std::endl;
				start_model->vertices = (Vertex*)malloc(start_model->vertex_count * sizeof(Vertex));
				start_model->texture_coords = (TextureCoord*)malloc(start_model->texture_count * sizeof(TextureCoord));
				start_model->normals = (Normal*)malloc(start_model->vertex_count * sizeof(Normal));
				start_model->faces = (Face*)malloc(start_model->face_count * sizeof(Face));
				std::cout << "				vertex_count : " << start_model->vertex_count << "개 | texture_count : "
					<< start_model->texture_count << "개 | normal_count : " << start_model->normal_count 
					<< "개 | face_count : " << start_model->face_count << std::endl;
				std::cout << "			[ 현재 모델에 메모리 할당 성공 ]" << std::endl;

				Model* new_model = (Model*)calloc(1, sizeof(Model));

				start_model->next = new_model;
				start_model = start_model->next;

				std::cout << "		[ 다음 모델에 메모리 준비 성공 ]" << std::endl;
			}
		}
		else if (line[0] == 'v' && line[1] == ' ') {
			start_model->vertex_count++; 
		}
		else if (line[0] == 'v' && line[1] == 't') {
			start_model->texture_count++; 
		}
		else if (line[0] == 'v' && line[1] == 'n') {
			start_model->normal_count++; 
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			start_model->face_count++; 
		}
	}
	// 마지막 모델에 메모리 할당
	std::cout << "			[ 현재 모델에 메모리 할당 시작 ]" << std::endl;
	start_model->vertices = (Vertex*)malloc(start_model->vertex_count * sizeof(Vertex));
	start_model->texture_coords = (TextureCoord*)malloc(start_model->texture_count * sizeof(TextureCoord));
	start_model->normals = (Normal*)malloc(start_model->vertex_count * sizeof(Normal));
	start_model->faces = (Face*)malloc(start_model->face_count * sizeof(Face));
	std::cout << "				vertex_count : " << start_model->vertex_count << "개 | texture_count : " << start_model->texture_count 
		<< "개 | normal_count : " << start_model->normal_count << "개 | face_count : " << start_model->face_count << std::endl;
	std::cout << "			[ 현재 모델에 메모리 할당 성공 ]" << std::endl;

	std::cout << "	[ 데이터 크기 계산 종료 ]" << std::endl;

	// MTL 파일 읽기
	if (mtl_filename[0] != '\0') {

		char mtl_path[MAX_LINE_LENGTH];
		snprintf(mtl_path, sizeof(mtl_path), "%s", mtl_filename);
		read_mtl_file(mtl_path, &materials, &material_count);
	}

	start_model = *model; // 처음 위치로 되돌리기
	int model_count = 0;
	size_t vertex_index = 0, texture_index = 0, normal_index = 0, face_index = 0;
	size_t vertex_count = 0, normal_count = 0; 

	// 두 번째 스캔 : 데이터 읽기
	std::cout << "	[ 데이터 읽기 시작 ]" << std::endl;
	fseek(file, 0, SEEK_SET);
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (line[0] == 'o' && line[1] == ' ') {
			if (start_model == *model && model_count == 0) {
				std::cout << "		[ 현재 모델에 데이터 삽입 시작 ]" << std::endl;
				model_count++;
				continue;
			}
			else {
				std::cout << "		[ 현재 모델에 데이터 삽입 종료 ]" << std::endl;
				start_model = start_model->next;
				vertex_count += vertex_index;
				normal_count += normal_index;
				vertex_index = 0; texture_index = 0; normal_index = 0; face_index = 0;

				std::cout << "		[ 현재 모델에 데이터 삽입 시작 ]" << std::endl;
			}
		}
		else if (line[0] == 'v' && line[1] == ' ') {
			sscanf_s(line + 2, "%f %f %f",
				&start_model->vertices[vertex_index].x,
				&start_model->vertices[vertex_index].y,
				&start_model->vertices[vertex_index].z);
			vertex_index++;
		}
		else if (line[0] == 'v' && line[1] == 't') {
			sscanf_s(line + 2, "%f %f",
				&start_model->texture_coords[texture_index].u,
				&start_model->texture_coords[texture_index].v);
			texture_index++;
		}
		else if (line[0] == 'v' && line[1] == 'n') {
			sscanf_s(line + 2, "%f %f %f",
				&start_model->normals[normal_index].x,
				&start_model->normals[normal_index].y,
				&start_model->normals[normal_index].z);
			normal_index++;
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			unsigned int v[4] = { 0 }, vt[4] = { 0 }, vn[4] = { 0 };
			int matches = sscanf_s(line + 2,
				"%u/%u/%u %u/%u/%u %u/%u/%u %u/%u/%u",
				&v[0], &vt[0], &vn[0],
				&v[1], &vt[1], &vn[1],
				&v[2], &vt[2], &vn[2],
				&v[3], &vt[3], &vn[3]);

			// Handle 3 vertices
			if (matches == 9) {
				start_model->faces[face_index].v1 = v[0] - vertex_count - 1;
				start_model->faces[face_index].vt1 = vt[0] - 1;
				start_model->faces[face_index].vn1 = vn[0] - normal_count - 1;

				start_model->faces[face_index].v2 = v[1] - vertex_count - 1;
				start_model->faces[face_index].vt2 = vt[1] - 1;
				start_model->faces[face_index].vn2 = vn[1] - normal_count - 1;

				start_model->faces[face_index].v3 = v[2] - vertex_count - 1;
				start_model->faces[face_index].vt3 = vt[2] - 1;
				start_model->faces[face_index].vn3 = vn[2] - normal_count - 1;

				// Set 4th vertex to be the same as the 2nd
				start_model->faces[face_index].v4 = v[1] - vertex_count - 1;
				start_model->faces[face_index].vt4 = vt[1] - 1;
				start_model->faces[face_index].vn4 = vn[1] - normal_count - 1;
			}
			// Handle 4 vertices
			else if (matches == 12) {
				start_model->faces[face_index].v1 = v[0] - vertex_count - 1;
				start_model->faces[face_index].vt1 = vt[0] - 1;
				start_model->faces[face_index].vn1 = vn[0] - normal_count - 1;

				start_model->faces[face_index].v2 = v[1] - vertex_count - 1;
				start_model->faces[face_index].vt2 = vt[1] - 1;
				start_model->faces[face_index].vn2 = vn[1] - normal_count - 1;

				start_model->faces[face_index].v3 = v[2] - vertex_count - 1;
				start_model->faces[face_index].vt3 = vt[2] - 1;
				start_model->faces[face_index].vn3 = vn[2] - normal_count - 1;

				start_model->faces[face_index].v4 = v[3] - vertex_count - 1;
				start_model->faces[face_index].vt4 = vt[3] - 1;
				start_model->faces[face_index].vn4 = vn[3] - normal_count - 1;
			}
			else {
				std::cerr << "Unsupported face format: " << line << std::endl;
				continue;
			}

			face_index++;
		}
		else if (strncmp(line, "usemtl", 6) == 0) {
			sscanf_s(line + 7, "%255s", current_material, (unsigned)_countof(current_material));

			// 재질 정보 할당
			if (materials && material_count > 0) {
				for (size_t i = 0; i < material_count; ++i) {
					if (strcmp(materials[i].name, current_material) == 0) {
						start_model->material = materials[i];
						break;
					}
				}
			}
		}
	}
	std::cout << "		[ 현재 모델에 데이터 삽입 종료 ]" << std::endl;

	start_model = *model;
	while (start_model != NULL) {
		std::cout << "		[ 현재 모델에 노멀값과 정점 값 일치화 시작 ]" << std::endl;
		match_normal_and_vertex(&start_model);
		start_model = start_model->next;
		std::cout << "		[ 현재 모델에 노멀값과 정점 값 일치화 종료 ]" << std::endl;
	}

	std::cout << "	[ 데이터 읽기 종료 ]" << std::endl;

	fclose(file);

	// 재질 정보 해제
	if (materials) {
		free(materials);
	}

	std::cout << "=====[Read Obj File End]================================================================================================" << std::endl;
}