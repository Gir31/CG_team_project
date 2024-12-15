#include "GL_My_header.h"
#include "obj_reader.h"
#include <time.h>
#pragma comment(lib, "winmm")
#include <mmsystem.h>
#include <windows.h>
//========================================================
#define PI 3.141592f
#define MAX_LANE 7
#define MAX_SPEED 700 // 최고 속도
#define MIN_SPEED 1 // 최저 속도
#define SOUND_FILE_NAME_WEE "Whoosh-4.wav"
#define SOUND_FILE_NAME_BOOM "Boom-Short.wav"
#define SOUND_FILE_NAME_CRUSH "Crash-Rock-Crash-RockImpact-02.wav"
#define SOUND_FILE_NAME_CRUSH2 "Crash-Glass-Crash-07.wav"//no
#define SOUND_FILE_NAME_BGM "champion-upbeat-motivation-advertising-rock-251285.wav"
#define SOUND_FILE_NAME_CLAXON "Claxon-2.wav"


typedef struct obstacle {

	glm::vec3 move;
	bool lane_change;
	float target_lane;
	float speed;
	int type;

	struct obstacle* next;
}OBSTACLE;

typedef struct item {
	glm::vec3 move;
	glm::vec3 rotate;
	float levitation;
	int type;

	struct item* next;
}ITEM;

OBSTACLE* obstacle_head = NULL;
ITEM* item_head = NULL;
//========================================================
// 원근 투영
PROJECTION projection = { 45.0f, 0.0f, 0.1f, 900.0f };
glm::vec3 spaceTrans = glm::vec3(0.0f, 0.0f, -2.0f);
//========================================================
char vertex[] = { "vertex.glsl" };
char fragment[] = { "fragment.glsl" };
unsigned int VBO, VAO;
GLuint shaderProgramID;
//========================================================
// 사용자 지정 변수
// 모델 변수
Model* main_car = NULL;
Model* obstacle_car[3] = { NULL, NULL, NULL };
Model* item[2] = { NULL, NULL };
Model* background = NULL;
size_t index_count = 0;

// 카메라 변수
glm::vec3 transCamera = glm::vec3(0.0f, 15.0f, 100.0f);
glm::vec3 rotateCamera = glm::vec3(0, 0, 0);

// 메인 자동차 변수
glm::vec3 rotate_car = glm::vec3(0, 0, 0);
glm::vec3 trans_car = glm::vec3(0, 0, 5.f);

GLfloat speed_value = 200, acceleration = -2;

GLboolean is_colliding = FALSE;
GLboolean have_shield = FALSE;

GLboolean dash = FALSE; clock_t dash_timer = clock(); 
GLint original_speed = 300; // speed_value의 기본값 저장

GLint curr_lane = 4, target_lane = 4; // 최소 레인 1, 최대 레인 7
GLfloat lane_coords[7] = { -15.f, -10.f, -5.f, 0.f, 5.f, 10.f, 15.f };

glm::vec3 hovering_item_rotate[3] = { glm::vec3(0, 0, 0), glm::vec3(0, 120, 0), glm::vec3(0, 240, 0)};


// 배경 변수
glm::vec3 trans_background[2] = { glm::vec3(0, -5.f, -500.f), glm::vec3(0, -5.f, -1400.f) };

float speed_bar[] = {
	-80.f, -20.f, 0.f, 0.f, 0.f, 1.f,
	-80.f, -34.f, 0.f, 0.f, 0.f, 1.f,
	80.f, -34.f, 0.f, 0.f, 0.f, 1.f,
	80.f, -20.f, 0.f, 0.f, 0.f, 1.f
};

float speed[] = {
	-5.5f, -20.f, 0.f, 0.f, 0.f, 1.f,
	-5.5f, -34.f, 0.f, 0.f, 0.f, 1.f,
	5.5f, -34.f, 0.f, 0.f, 0.f, 1.f,
	5.5f, -20.f, 0.f, 0.f, 0.f, 1.f
};

unsigned int transformLocation;

GLboolean left_button = FALSE;
GLboolean speed_flag = FALSE;
//========================================================
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid SpecialKeyboard(int key, int x, int y);
GLvoid TimerFunction(int value);
void make_shaderProgram();
GLvoid initBuffer(const Model* model);
GLvoid initBuffer_(const float* list, glm::vec3 color);
//========================================================
// 사용자 지정 함수
GLvoid draw_model(Model* model);

GLvoid cameraTranslation(glm::vec3 cameraTrans, glm::vec3 cameraRotate);
GLvoid speed_camera_move();

GLvoid move_background();
GLvoid move_car(); 
GLvoid spin_car();

// 장애물 관련 함수
GLvoid createObstacle();
GLvoid drawObstacle();
GLvoid removeObstacle(OBSTACLE* target);
GLvoid moveObstacle();

// 아이템 관련 함수
GLvoid createItem();
GLvoid drawItem();
GLvoid removeItem(ITEM* target);
GLvoid moveItem();

GLvoid hovering();
GLvoid hovering_around_car_item();
//========================================================

void ConvertToWideChar(const char* source, WCHAR* dest, size_t destSize) {
	MultiByteToWideChar(CP_ACP, 0, source, -1, dest, static_cast<int>(destSize));
}

// 효과음을 미리 로드하는 함수
void PreloadEffectSound(const char* soundFile, const char* alias) {
	WCHAR wideCommand[256];
	WCHAR wideSoundFile[256];

	// 멀티바이트 -> 유니코드 변환
	ConvertToWideChar(soundFile, wideSoundFile, 256);

	// 사운드 파일 미리 열기
	wsprintf(wideCommand, L"open \"%s\" type waveaudio alias %s", wideSoundFile, alias);
	mciSendString(wideCommand, NULL, 0, NULL);
}

// 미리 로드된 효과음을 재생하는 함수
void PlayPreloadedEffectSound(const char* alias) {
	WCHAR wideCommand[256];

	// 미리 열린 alias를 재생
	wsprintf(wideCommand, L"play %s from 0", alias);
	mciSendString(wideCommand, NULL, 0, NULL);
}

// 미리 로드된 효과음을 닫는 함수
void ClosePreloadedEffectSound(const char* alias) {
	WCHAR wideCommand[256];

	// alias 닫기
	wsprintf(wideCommand, L"close %s", alias);
	mciSendString(wideCommand, NULL, 0, NULL);
}



int main(int argc, char** argv)
{
	PlaySound(TEXT(SOUND_FILE_NAME_BGM), NULL, SND_ASYNC | SND_ALIAS| SND_LOOP);
	// 효과음 미리 로드
	PreloadEffectSound(SOUND_FILE_NAME_CLAXON, "claxon");
	PreloadEffectSound(SOUND_FILE_NAME_CRUSH, "crush");
	PreloadEffectSound(SOUND_FILE_NAME_WEE, "wee");

	// 시작 시 경적 소리 재생
	PlayPreloadedEffectSound("claxon");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800.0f, 800.0f);
	glutCreateWindow("Example1");

	glewExperimental = GL_TRUE;
	glewInit();

	make_shaderProgram();
	// 메인 차량
	read_obj_file_with_mtl("car_s.obj", &main_car);
	// 장애물 차량
	read_obj_file_with_mtl("car_k.obj", &obstacle_car[0]);
	read_obj_file_with_mtl("car_r.obj", &obstacle_car[1]);
	read_obj_file_with_mtl("car_t.obj", &obstacle_car[2]);
	// 아이템
	read_obj_file_with_mtl("coin.obj", &item[0]);
	read_obj_file_with_mtl("shield.obj", &item[1]);
	// 배경
	read_obj_file_with_mtl("back_color.obj", &background);


	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutTimerFunc(10, TimerFunction, 1);

	glutMainLoop();

	// 프로그램 종료 시 효과음 닫기
	ClosePreloadedEffectSound("claxon");
	ClosePreloadedEffectSound("crush");
	ClosePreloadedEffectSound("wee");
}

GLvoid drawScene() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Perspective_Projection_Transformation(projection, spaceTrans, shaderProgramID);
	cameraTranslation(transCamera, rotateCamera);

	glUseProgram(shaderProgramID);

	if (transformLocation == NULL)
		transformLocation = glGetUniformLocation(shaderProgramID, "model");

	// 주인공(car_s) 그리기
	glm::mat4 main_car_model = glm::mat4(1.f);
	main_car_model = translation_shape(trans_car) * rotate_shape(rotate_car);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(main_car_model));
	draw_model(main_car);

	for (int i = 0; i < 2; i++) {
		glm::mat4 background_model = glm::mat4(1.f);
		background_model = translation_shape(trans_background[i]) * rotate_shape(glm::vec3(0, 90, 0)) * scaling_shape(glm::vec3(2, 2, 2));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(background_model));
		draw_model(background);
	}

	drawObstacle();
	drawItem();

	if(have_shield) hovering_around_car_item();

	Orthogoanl_Projection_Transformation(100.f, shaderProgramID);

	glm::mat4 bar = glm::mat4(1.f);
	bar = translation_shape(glm::vec3(0, 0, 100.f));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(bar));
	initBuffer_(speed_bar, glm::vec3(1, 1, 1));
	glDrawArrays(GL_QUADS, 0, 24);

	glutSwapBuffers();
}


GLvoid Reshape(int w, int h)
{
	projection.Aspect = w / h;

	glViewport(0, 0, w, h);
}

void make_shaderProgram()
{
	make_VertexShaders(vertex);
	make_FragmentShaders(fragment);

	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgramID);
}

GLvoid Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
	}
	glutPostRedisplay();
}

void Motion(int x, int y) {
	if (left_button) {

	}
	glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {

	case 'q': case 'Q':
		glutLeaveMainLoop();
	}
	glutPostRedisplay();
}

GLvoid SpecialKeyboard(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		if (curr_lane > 1 && target_lane == curr_lane && !is_colliding) { // 레인 제한
			target_lane -= 1;
			rotate_car.y = 0;
		}
		break;
	case GLUT_KEY_RIGHT:
		if (curr_lane < 7 && target_lane == curr_lane && !is_colliding) { // 레인 제한
			target_lane += 1;
			rotate_car.y = 0;
		}
		break;
	case GLUT_KEY_UP:
		speed_flag = TRUE;
		break;
	case GLUT_KEY_DOWN:
		speed_value -= 4;
		break;
	}
	glutPostRedisplay();
}


GLvoid TimerFunction(int value) {
	static int obstacle_timer = 0;
	static int item_timer = 0;

	// 장애물 생성 주기
	obstacle_timer++;
	if (obstacle_timer >= 50) {
		// 최대 4개 이상 유지
		int count = 0;
		OBSTACLE* curr = obstacle_head;
		while (curr != NULL) {
			count++;
			curr = curr->next;
		}

		if (count < 10) {
			createObstacle();
		}

		obstacle_timer = 0;
	}

	// 아이템 생성 주기
	item_timer++;
	if (item_timer >= 300) {
		// 최대 4개 이상 유지
		int count = 0;
		ITEM* curr = item_head;
		while (curr != NULL) {
			count++;
			curr = curr->next;
		}

		if (count < 2) {
			createItem();
		}

		item_timer = 0;
	}

	speed_camera_move();
	
	if (is_colliding) spin_car();
	else move_car();

	if (have_shield) hovering();

	if (dash) {if ((double)(clock() - dash_timer) / CLOCKS_PER_SEC > 5) dash = FALSE;}
	
	move_background();
	moveObstacle();
	moveItem();

	glutPostRedisplay();
	glutTimerFunc(10, TimerFunction, 1);
}

GLvoid initBuffer(const Model* model) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, model->face_count * 24 * sizeof(float), model->vertex_normal_list, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); //--- 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); //--- 노말 속성
	glEnableVertexAttribArray(1);

	glUseProgram(shaderProgramID);
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos"); //--- lightPos 값 전달: (0.0, 0.0, 5.0);
	glUniform3f(lightPosLocation, 0.0, 100.0, 0.0);
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor"); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
	glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos"); //--- viewPos 값 전달: 카메라 위치
	glUniform3f(viewPosLocation, transCamera.x, transCamera.y, transCamera.z);
	unsigned int objectColorLocation = glGetUniformLocation(shaderProgramID, "objectColor"); //--- objectcolor 값 전달: 오브젝트 컬러
	glUniform3f(objectColorLocation, 1, 1, 1);

	unsigned int material_ambient = glGetUniformLocation(shaderProgramID, "Ka"); //--- model의 ambient 값 전달
	glUniform3f(material_ambient, model->material.Ka[0], model->material.Ka[1], model->material.Ka[2]);
	unsigned int material_diffuse = glGetUniformLocation(shaderProgramID, "Kd"); //--- model의 diffuse 값 전달
	glUniform3f(material_diffuse, model->material.Kd[0], model->material.Kd[1], model->material.Kd[2]);
	unsigned int material_specular = glGetUniformLocation(shaderProgramID, "Ks"); //--- model의 specular 값 전달
	glUniform3f(material_specular, model->material.Ks[0], model->material.Ks[1], model->material.Ks[2]);
}

GLvoid initBuffer_(const float* list, glm::vec3 color) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), list, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); //--- 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); //--- 노말 속성
	glEnableVertexAttribArray(1);

	glUseProgram(shaderProgramID);
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos"); //--- lightPos 값 전달: (0.0, 0.0, 5.0);
	glUniform3f(lightPosLocation, 0.0, 0.0, 150.0);
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor"); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
	glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos"); //--- viewPos 값 전달: 카메라 위치
	glUniform3f(viewPosLocation, transCamera.x, transCamera.y, transCamera.z);
	unsigned int objectColorLocation = glGetUniformLocation(shaderProgramID, "objectColor"); //--- objectcolor 값 전달: 오브젝트 컬러
	glUniform3f(objectColorLocation, color.x, color.y, color.z);

	unsigned int material_ambient = glGetUniformLocation(shaderProgramID, "Ka"); //--- model의 ambient 값 전달
	glUniform3f(material_ambient, 1, 1, 1);
	unsigned int material_diffuse = glGetUniformLocation(shaderProgramID, "Kd"); //--- model의 diffuse 값 전달
	glUniform3f(material_diffuse, 1, 1, 1);
	unsigned int material_specular = glGetUniformLocation(shaderProgramID, "Ks"); //--- model의 specular 값 전달
	glUniform3f(material_specular, 1, 1, 1);
}

GLvoid draw_speed() {
	int speed_count = (int)speed_value / 50;

	for (int i = 0; i < speed_count; i++) {

	}
	glm::mat4 bar = glm::mat4(1.f);
	bar = translation_shape(glm::vec3(0, 0, 100.f));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(bar));
	initBuffer_(speed_bar, glm::vec3(1, 1, 1));
	glDrawArrays(GL_QUADS, 0, 24);
}

GLvoid draw_model(Model* model) {
	Model* curr_model = model;

	while (curr_model != NULL) {
		initBuffer(curr_model);

		glDrawArrays(GL_QUADS, 0, curr_model->face_count * 24);

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);

		curr_model = curr_model->next;
	}
}

GLvoid cameraTranslation(glm::vec3 cameraTrans, glm::vec3 cameraRotate) {
	glm::mat4 view = glm::mat4(1.0f);
	view = camera_locate(cameraTrans, glm::vec3(0, 0, 0)) * rotate_camera(cameraRotate);

	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
}

GLvoid speed_camera_move() {
	acceleration = -1;

	if (speed_value < 250) acceleration = 0.5;

	if (dash) acceleration = 5;

	speed_value += acceleration;

	if (speed_value >= MAX_SPEED) speed_value = MAX_SPEED; 
	else if (speed_value < MIN_SPEED) speed_value = 1;

	transCamera.z = ((GLfloat)speed_value / 4);
	transCamera.y = transCamera.z * 3 / 10;
	trans_car.z = ((GLfloat)speed_value / 50);
}

GLvoid move_car() {
	if (curr_lane > target_lane) {
		
		if (lane_coords[target_lane - 1] >= trans_car.x) {
			rotate_car.y -= 5;
			rotateCamera.z += 0.5;
			if (rotate_car.y <= 0) {
				curr_lane -= 1;
				rotate_car.y = 0;
				rotateCamera.z = 0;
			}
		}
		else {
			trans_car.x -= 0.5f;
			rotate_car.y += 5;
			rotateCamera.z -= 0.5;
		}
	}
	else if (curr_lane < target_lane) {	
		if (lane_coords[target_lane - 1] <= trans_car.x) {
			rotate_car.y += 5;
			rotateCamera.z -= 0.5;
			if (rotate_car.y >= 0) {
				curr_lane += 1;
				rotate_car.y = 0;
				rotateCamera.z = 0;
			}
		}
		else {
			trans_car.x += 0.5f;
			rotate_car.y -= 5;
			rotateCamera.z += 0.5;
		}
	}
}

GLvoid spin_car() {
	static float rc_value = 2;

	rotate_car.y += 18;
	rotateCamera.x += rc_value;

	if ((int)(rotate_car.y) % 90 == 0) rc_value *= -1;

	if (rotate_car.y / 360 >= 2) {
		is_colliding = FALSE;
		rotate_car.y = 0;
		rotateCamera.x = 0;
	}
}

GLvoid move_background() {
	for (int i = 0; i < 2; i++) {
		trans_background[i].z += speed_value / 100.f;

		if (trans_background[i].z >= 400) trans_background[i].z = -1400.f;
	}
}

GLvoid createObstacle() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> lane_dist(1, MAX_LANE);
	std::uniform_int_distribution<int> car_type(0, 2);
	std::uniform_int_distribution<int> speed_dist(50, 150);
	std::uniform_real_distribution<float> z_dist(-900.0f, -700.0f); // 장애물 생성 범위를 -900 ~ -700으로 확장

	OBSTACLE* newObstacle = (OBSTACLE*)malloc(sizeof(OBSTACLE));
	newObstacle->next = NULL;
	newObstacle->move.x = ((lane_dist(gen) - 4) * 5.f); // 레인에 맞춰 x 좌표 설정
	newObstacle->move.y = 0.0f;
	newObstacle->move.z = z_dist(gen); // 카메라 밖에서 생성
	newObstacle->speed = speed_dist(gen);
	newObstacle->type = car_type(gen);
	newObstacle->lane_change = FALSE;

	// 링크드리스트로 추가
	OBSTACLE* curr = obstacle_head;
	if (curr == NULL) {
		obstacle_head = newObstacle;
	}
	else {
		while (curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = newObstacle;
	}
}

GLvoid drawObstacle() {
	OBSTACLE* curr = obstacle_head;
	unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");

	while (curr != NULL) {
		// 장애물의 변환 행렬 설정
		glm::mat4 obstacle_model = glm::mat4(1.f);
		obstacle_model = translation_shape(curr->move);
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(obstacle_model));

		// 장애물 모델을 사용하여 그리기
		draw_model(obstacle_car[curr->type]);

		curr = curr->next;
	}
}

GLvoid removeObstacle(OBSTACLE* target) {
	OBSTACLE* curr = obstacle_head;

	if (curr == target) {
		obstacle_head = curr->next;
		free(curr);
	}
	else {
		while (curr->next != target) { curr = curr->next; }

		curr->next = target->next;
		free(target);
	}

}

GLboolean checkCollision(glm::vec3 car, glm::vec3 obstacle) {
	float distance = glm::length(car - obstacle);
	return distance < 5.f; // 충돌 판정 거리
}

GLvoid reckless_driving(OBSTACLE* obstacle) {
	OBSTACLE* curr = obstacle_head;

	while (curr != NULL) {
		if (curr != obstacle && checkCollision(obstacle->move, curr->move) && !obstacle->lane_change) {
			obstacle->lane_change = TRUE;

			if (obstacle->move.x == -15.f)  obstacle->target_lane = -10.f;
			else if (obstacle->move.x == 15.f) obstacle->target_lane = 10.f;
			else {
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_int_distribution<int> random_int(0, 1);

				random_int(gen) ? obstacle->target_lane = obstacle->move.x -= 5.f : obstacle->target_lane = obstacle->move.x += 5.f;
			}

			break;
		}

		curr = curr->next;
	}
}

GLvoid moveObstacle() {
	OBSTACLE* curr = obstacle_head;

	while (curr != NULL) {
		curr->move.z += speed_value / curr->speed;

		reckless_driving(curr);

		if (curr->lane_change) {
			if (curr->move.x > curr->target_lane) {
				curr->move.x -= 0.1f;
			}
			else if (curr->move.x < curr->target_lane) {
				curr->move.x += 0.1f;
			}
			else {
				curr->lane_change = FALSE;
			}
		}

		// 충돌 검사
		if (!is_colliding && checkCollision(trans_car, curr->move)) {
			printf("Collision detected!\n");

			OBSTACLE* target = curr;
			curr = curr->next;
			removeObstacle(target);

			if (!have_shield) {
				rotate_car.y = 0;
				is_colliding = TRUE;
				speed_value -= 150;

				PlayPreloadedEffectSound("crush");
			}
			else have_shield = FALSE;
		}

		// 화면 넘어가는 장애물 제거
		if (curr->move.z > 50.0f) {
			OBSTACLE* target = curr;
			curr = curr->next;
			removeObstacle(target);
		}
		else {
			curr = curr->next;
		}
	}
}

GLvoid createItem() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> lane_dist(1, MAX_LANE);
	std::uniform_int_distribution<int> item_type(0, 1);
	std::uniform_real_distribution<float> z_dist(-900.0f, -700.0f); // 장애물 생성 범위를 -900 ~ -700으로 확장

	ITEM* newItem = (ITEM*)malloc(sizeof(ITEM)); 
	newItem->next = NULL;
	newItem->move.x = ((lane_dist(gen) - 4) * 5.f); // 레인에 맞춰 x 좌표 설정
	newItem->move.y = 0.0f;
	newItem->move.z = z_dist(gen); // 카메라 밖에서 생성
	newItem->rotate = glm::vec3(0, 0, 0);
	newItem->levitation = 0.1f;
	newItem->type = item_type(gen);

	// 링크드리스트로 추가
	ITEM* curr = item_head;
	if (curr == NULL) {
		item_head = newItem;
	}
	else {
		while (curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = newItem;
	}
}

GLvoid drawItem() {
	ITEM* curr = item_head;
	unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");

	while (curr != NULL) {
		// 아이템의 변환 행렬 설정
		glm::mat4 item_model = glm::mat4(1.f);
		item_model = translation_shape(curr->move) * rotate_shape(curr->rotate) * scaling_shape(glm::vec3(5.f, 5.f, 5.f));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(item_model));

		// 아이템 모델을 사용하여 그리기
		draw_model(item[curr->type]);

		curr = curr->next;
	}
}

GLvoid removeItem(ITEM* target) {
	ITEM* curr = item_head;

	if (curr == target) {
		item_head = curr->next;
		free(curr);
	}
	else {
		while (curr->next != target) { curr = curr->next; }

		curr->next = target->next;
		free(target);
	}

}

GLvoid moveItem() {
	ITEM* curr = item_head;

	while (curr != NULL) {
		curr->move.z += speed_value / 100;
		curr->move.y += curr->levitation;
		curr->rotate.y = (int)(curr->rotate.y + 3.f) % 360;

		if ((int)(curr->rotate.y) % 120 == 0) {
			curr->levitation *= -1;
		}

		// 충돌 검사
		if (!is_colliding && checkCollision(trans_car, curr->move)) {
			printf("Acquire item!\n");

			switch (curr->type) {
			case 0:
				dash = TRUE;
				PlayPreloadedEffectSound("wee");
				dash_timer = clock();
				break;
			case 1:
				have_shield = TRUE;
				PlayPreloadedEffectSound("wee");

				break;
			}

			ITEM* target = curr;
			if (curr->next != NULL) {
				curr = curr->next;
				removeItem(target);
			}
			else {
				removeItem(target);
				break;
			}
		}

		// 화면 넘어가는 아이템 제거
		if (curr->move.z > 50.0f) {
			ITEM* target = curr;
			curr = curr->next;
			removeItem(target);
		}
		else {
			curr = curr->next;
		}
	}
}

GLvoid hovering() {
	for (int i = 0; i < 3; i++) {
		hovering_item_rotate[i].y += 3;
	}
}

GLvoid hovering_around_car_item() {

	glm::mat4 item_model = glm::mat4(1.f);
	item_model = translation_shape(trans_car) * rotate_shape(hovering_item_rotate[0]) * translation_shape(glm::vec3(0, 0, 4.f)) * scaling_shape(glm::vec3(2.f, 2.f, 2.f));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(item_model));
	draw_model(item[1]);

	item_model = translation_shape(trans_car) * rotate_shape(hovering_item_rotate[1]) * translation_shape(glm::vec3(0, 0, 4.f)) * scaling_shape(glm::vec3(2.f, 2.f, 2.f));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(item_model));
	draw_model(item[1]);

	item_model = translation_shape(trans_car) * rotate_shape(hovering_item_rotate[2]) * translation_shape(glm::vec3(0, 0, 4.f)) * scaling_shape(glm::vec3(2.f, 2.f, 2.f));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(item_model));
	draw_model(item[1]);
}