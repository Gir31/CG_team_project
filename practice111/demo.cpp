#include "GL_My_header.h"
#include "obj_reader.h"
//#include "prac.h"
//========================================================
#define PI 3.141592f
#define MAX_LANE 7
#define MAX_SPEED 300 // 최고 속도
#define MIN_SPEED 150 // 최저 속도

typedef struct obstacle {

	glm::vec3 move;

	struct obstacle* next;
}OBSTACLE;

OBSTACLE* start = NULL;
//========================================================
// 원근 투영
PROJECTION projection = { 45.0f, 0.0f, 0.1f, 500.0f };
glm::vec3 spaceTrans = glm::vec3(0.0f, 0.0f, -2.0f);
//========================================================
char vertex[] = { "vertex.glsl" };
char fragment[] = { "fragment.glsl" };
// unsigned int VBO[2], VAO, EBO;
GLuint shaderProgramID;
//========================================================
// 사용자 지정 변수
// 모델 변수
Model* main_car = NULL;
Model* background = NULL;
size_t index_count = 0;

// 카메라 변수
glm::vec3 transCamera = glm::vec3(0.0f, 20.0f, 50.0f);
glm::vec3 rotateCamera = glm::vec3(0, 0, 0);

// 메인 자동차 변수
glm::vec3 rotate_car = glm::vec3(0, 0, 0);
glm::vec3 trans_car = glm::vec3(0, 0, 10.f);

GLint speed_value = 200;
GLint acceleration = -2;

// 배경 변수
glm::vec3 trans_background[2] = { glm::vec3(0, -10.f, -200.f), glm::vec3(0, -10.f, -700.f) };

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

GLvoid drawScene()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Perspective_Projection_Transformation(projection, spaceTrans, shaderProgramID);
	cameraTranslation(transCamera, rotateCamera);

	glUseProgram(shaderProgramID);

	if (transformLocation == NULL) transformLocation = glGetUniformLocation(shaderProgramID, "model");
	
	glm::mat4 main_car_model = glm::mat4(1.f);
	main_car_model = translation_shape(trans_car) * rotate_shape(rotate_car);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(main_car_model));
	draw_model(main_car);

	glm::mat4 background_model = glm::mat4(1.f);
	background_model = translation_shape(trans_background[0]) * rotate_shape(glm::vec3(0, 90, 0));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(background_model));
	draw_model(background);

	glm::mat4 background_model2 = glm::mat4(1.f);
	background_model2 = translation_shape(trans_background[1]) * rotate_shape(glm::vec3(0, 90, 0));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(background_model2));
	draw_model(background);


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

		break;
	case GLUT_KEY_RIGHT:

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
	glutPostRedisplay();

	speed_camera_move(); 
	move_background();

	glutTimerFunc(10, TimerFunction, 1);
}

GLvoid initBuffer(const Model* model) {
	unsigned int VBO, VAO; 

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

	if (speed_value >= MAX_SPEED || speed_value <= MIN_SPEED) acceleration = 0; // 속도가 최저 혹은 최소에 도달할 때 가속 값을 0으로 변경

	if (speed_flag && speed_value < MAX_SPEED - 8) {
		acceleration = 8;
		speed_flag = FALSE;
	}
	
	speed_value += acceleration;

	transCamera.z = ((GLfloat)speed_value / 4); 
}

GLvoid move_background() {
	for (int i = 0; i < 2; i++) {
		trans_background[i].z += speed_value / 100.f;

		if (trans_background[i].z >= 300)trans_background[i].z = -700.f;
	}
}

// 장애물 생성 코드
GLvoid createObstacle() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(1, 7);

	OBSTACLE* newObstacle = (OBSTACLE*)malloc(sizeof(OBSTACLE));

	newObstacle->next = NULL;
	newObstacle->move.x = ((dis(gen) % 7) - 3) * 1.2f; // x값만 랜덤으로 생성, 레인 수에 맞게 결정
	newObstacle->move.y = 0.f;
	newObstacle->move.z = -5.f;

	// 링크드리스트로 구현
	OBSTACLE* curr = start;
	if (curr == NULL) start = newObstacle;
	else {
		while (curr->next != NULL) { curr = curr->next; }

		curr->next = newObstacle;
	}
}

GLvoid drawObstacle() {
	OBSTACLE* curr = start;
	unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");

	while (curr != NULL) {
		// 각 장애물마다 고유 값으로 그리기
		glm::mat4 obstacle = glm::mat4(1.f);
		obstacle = translation_shape(curr->move);
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(obstacle));

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

GLvoid moveObstacle() {
	OBSTACLE* curr = start;

	while (curr != NULL) {
		curr->move.z += 0.1f;

		// 일정 위치 도달시 삭제
		if (curr->move.z > 10.f) { 
			OBSTACLE* target = curr;
			curr = curr->next;

			removeObstacle(target);
		}

		if(curr != NULL) curr = curr->next;
	}
}