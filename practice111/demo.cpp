#include "GL_My_header.h"
#include "obj_reader.h"
//#include "prac.h"
//========================================================
#define PI 3.141592f
#define MAX_LANE 7
#define MAX_SPEED 500 // 최고 속도
#define MIN_SPEED 1 // 최저 속도

typedef struct obstacle {

	glm::vec3 move;
	bool lane_change;
	float target_lane;
	float speed;
	int type;

	struct obstacle* next;
}OBSTACLE;

OBSTACLE* start = NULL;
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
GLint original_speed = 300; // speed_value의 기본값 저장

GLint curr_lane = 4, target_lane = 4; // 최소 레인 1, 최대 레인 7
GLfloat lane_coords[7] = { -15.f, -10.f, -5.f, 0.f, 5.f, 10.f, 15.f };


// 배경 변수
glm::vec3 trans_background[2] = { glm::vec3(0, -5.f, -500.f), glm::vec3(0, -5.f, -1400.f) };

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
//========================================================
// 사용자 지정 함수
GLvoid draw_model(Model* model);

GLvoid cameraTranslation(glm::vec3 cameraTrans, glm::vec3 cameraRotate);
GLvoid speed_camera_move();

GLvoid move_background();
GLvoid move_car(); 
GLvoid spin_car();

GLvoid createObstacle();
GLvoid drawObstacle();
GLvoid removeObstacle(OBSTACLE* target);
GLvoid moveObstacle();
//========================================================

int main(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800.0f, 800.0f);
	glutCreateWindow("Example1");

	glewExperimental = GL_TRUE;
	glewInit();

	make_shaderProgram();
	read_obj_file_with_mtl("car_s.obj", &main_car);
	read_obj_file_with_mtl("car_k.obj", &obstacle_car[0]);
	read_obj_file_with_mtl("car_r.obj", &obstacle_car[1]);
	read_obj_file_with_mtl("car_t.obj", &obstacle_car[2]);
	read_obj_file_with_mtl("back_color.obj", &background);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutTimerFunc(10, TimerFunction, 1);

	glutMainLoop();
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

	// 장애물 생성 주기
	obstacle_timer++;
	if (obstacle_timer >= 50) {
		// 최대 4개 이상 유지
		int count = 0;
		OBSTACLE* curr = start;
		while (curr != NULL) {
			count++;
			curr = curr->next;
		}

		if (count < 10) {
			createObstacle();
		}

		obstacle_timer = 0;
	}

	speed_camera_move();
	
	if (is_colliding) spin_car();
	else move_car();
	
	move_background();
	moveObstacle();

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

	unsigned int material_ambient = glGetUniformLocation(shaderProgramID, "Ka"); //--- model의 ambient 값 전달
	glUniform3f(material_ambient, model->material.Ka[0], model->material.Ka[1], model->material.Ka[2]);
	unsigned int material_diffuse = glGetUniformLocation(shaderProgramID, "Kd"); //--- model의 diffuse 값 전달
	glUniform3f(material_diffuse, model->material.Kd[0], model->material.Kd[1], model->material.Kd[2]);
	unsigned int material_specular = glGetUniformLocation(shaderProgramID, "Ks"); //--- model의 specular 값 전달
	glUniform3f(material_specular, model->material.Ks[0], model->material.Ks[1], model->material.Ks[2]);
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

	if (speed_value >= MAX_SPEED) acceleration = 0; // 속도가 최저 혹은 최소에 도달할 때 가속 값을 0으로 변경
	else if (speed_value < MIN_SPEED) speed_value = 1;

	speed_value += acceleration;

	transCamera.z = ((GLfloat)speed_value / 4);
	transCamera.y = transCamera.z * 3 / 10;
	trans_car.z = ((GLfloat)speed_value / 50);

	printf("	Current Speed : %f km/h\n", speed_value);
}

GLvoid move_car() {
	if (curr_lane > target_lane) {
		
		if (lane_coords[target_lane - 1] >= trans_car.x) {
			rotate_car.y -= 5;
			if (rotate_car.y <= 0) {
				curr_lane -= 1;
				rotate_car.y = 0;
			}
		}
		else {
			trans_car.x -= 0.5f;
			rotate_car.y += 5;
		}
	}
	else if (curr_lane < target_lane) {	
		if (lane_coords[target_lane - 1] <= trans_car.x) {
			rotate_car.y += 5;
			if (rotate_car.y >= 0) {
				curr_lane += 1;
				rotate_car.y = 0;
			}
		}
		else {
			trans_car.x += 0.5f;
			rotate_car.y -= 5;
		}
	}
}

GLvoid spin_car() {
	rotate_car.y += 36;

	if (rotate_car.y / 360 >= 2) {
		is_colliding = FALSE;
		rotate_car.y = 0;
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
	OBSTACLE* curr = start;
	if (curr == NULL) {
		start = newObstacle;
	}
	else {
		while (curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = newObstacle;
	}
}


GLvoid drawObstacle() {
	OBSTACLE* curr = start;
	unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");

	while (curr != NULL) {
		// 장애물의 변환 행렬 설정
		glm::mat4 obstacle_model = glm::mat4(1.f);
		obstacle_model = translation_shape(curr->move);
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(obstacle_model));

		// 모델(car_k.obj)을 사용하여 그리기
		draw_model(obstacle_car[curr->type]);

		curr = curr->next;
	}
}



GLvoid removeObstacle(OBSTACLE* target) {
	OBSTACLE* curr = start;

	if (curr == target) {
		start = curr->next;
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
	OBSTACLE* curr = start;

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
	OBSTACLE* curr = start;

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

			rotate_car.y = 0;
			is_colliding = TRUE;
			speed_value -= 150;
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