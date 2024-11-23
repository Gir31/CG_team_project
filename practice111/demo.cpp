#include "GL_My_header.h"
#include "obj_reader.h"
//========================================================
#define PI 3.141592f
#define MAX_LANE 7

typedef struct obstacle {

	glm::vec3 move;

	struct obstacle* next;
}OBSTACLE;

OBSTACLE* start = NULL;
//========================================================
// 원근 투영
PROJECTION projection = { 45.0f, 0.0f, 0.1f, 50.0f };
glm::vec3 spaceTrans = glm::vec3(0.0f, 0.0f, -2.0f);
//========================================================
char vertex[] = { "vertex.glsl" };
char fragment[] = { "fragment.glsl" };
GLuint vao, vbo[2];
GLuint shaderProgramID;
//========================================================
// 사용자 지정 변수
Model cube;

glm::vec3 transCamera = glm::vec3(0.0f, 5.0f, 10.0f);
glm::vec3 rotateCamera = glm::vec3(0, 0, 0);

glm::vec3 rotateCube = glm::vec3(0, 0, 0);
glm::vec3 transCube = glm::vec3(0, 0, 5.f);

GLfloat rotateValue = 1.f;
GLfloat transValue = 0.02f;

GLint moveCount = 0;
GLint lane = 4;
GLint targetLane = 4;

clock_t start_time;

GLboolean mFlag = FALSE;
GLboolean left_button = FALSE;
GLboolean timeSwitch = FALSE;
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
GLvoid cameraTranslation(glm::vec3 cameraTrans, glm::vec3 cameraRotate);
GLvoid moveCube(GLint count);

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

	unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");

	glUseProgram(shaderProgramID);

	read_obj_file("cube.obj", &cube);
	initBuffer(&cube);

	glm::mat4 moveCube = glm::mat4(1.f);
	moveCube = translation_shape(transCube) * rotate_shape(rotateCube);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(moveCube));

	glBindVertexArray(vao); 
	glDrawElements(GL_TRIANGLES, cube.face_count * 3, GL_UNSIGNED_INT, 0); 
	glBindVertexArray(0); 

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
		// target이 맨 왼쪽을 가르키지 않을 경우 
		if (targetLane != 1) {
			moveCount -= 1;
			mFlag = TRUE;
			targetLane -= 1;

			if(moveCount == 0) rotateValue = 1.f;
		}
		
		break;
	case GLUT_KEY_RIGHT:
		// target이 맨 오른쪽을 가르키지 않을 경우 
		if (targetLane != MAX_LANE) {
			moveCount += 1;
			mFlag = TRUE;
			targetLane += 1;

			if (moveCount == 0) rotateValue = -1.f;
		}
		
		break;
	case GLUT_KEY_UP:
		transCamera.z += 0.1f;
		break;
	case GLUT_KEY_DOWN:
		transCamera.z -= 0.1f;
		std::cout << transCamera.z << std::endl;
		break;
	}
	glutPostRedisplay();
}

GLvoid TimerFunction(int value) {
	glutPostRedisplay();

	if (!timeSwitch) {
		start_time = clock();
		timeSwitch = TRUE;
	}

	if(moveCount) moveCube(moveCount);
	moveObstacle();

	// 2초마다 장애물 생성
	if (clock() - start_time > 2000) { 
		start_time = clock();
		timeSwitch = FALSE;

		createObstacle();
	}

	glutTimerFunc(10, TimerFunction, 1);
}

GLvoid initBuffer(const Model* model) {
	GLuint ebo;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, model->vertex_count * sizeof(Vertex), model->vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->face_count * sizeof(Face), model->faces, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); 
} 

GLvoid cameraTranslation(glm::vec3 cameraTrans, glm::vec3 cameraRotate) {
	glm::vec3 zeroPoint = glm::vec3(0, 0, 0);
	glm::mat4 view = glm::mat4(1.0f);
	view = camera_locate(cameraTrans, zeroPoint) * rotate_camera(cameraRotate);

	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
}

GLvoid moveCube(GLint count) {
	static GLboolean rFlag = FALSE;

	// 카운트가 0 미만이면 왼쪽 이동, 아니면 오른쪽 이동
	if (count < 0) {
		rotateCube.y += rotateValue;
		transCube.x -= transValue;

		if (rotateCube.y <= -30.f || rotateCube.y >= 30.f) {
			rotateValue *= -1.f;
			rFlag = TRUE;
		}

		if (rFlag && rotateCube.y == 0.f) {
			rFlag = FALSE;
			moveCount += 1;
			lane -= 1;

			if (mFlag) rotateValue *= -1;
		}
	}
	else {
		rotateCube.y += rotateValue;
		transCube.x += transValue;

		if (rotateCube.y <= -30.f || rotateCube.y >= 30.f) {
			rotateValue *= -1.f;
			rFlag = TRUE;
		}

		if (rFlag && rotateCube.y == 0.f) {
			rFlag = FALSE;
			moveCount -= 1;
			lane += 1;

			if (mFlag) rotateValue *= -1;
		}
	}

	// 목표했던 레인에 도달시 movecount를 0으로 초기화
	if (lane == targetLane) moveCount = 0;
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

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, cube.face_count * 3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

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