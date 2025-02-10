#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace std;
using namespace _3dgl;
using namespace glm;

// GLSL Objects (Shader Program)
C3dglProgram programBasic;
C3dglProgram programParticle;

// texture ids
GLuint idTexParticle;
GLuint idBufferVelocity;
GLuint idBufferStartTime;
// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)
// Particle System Params

const float PERIOD = 0.00075f;

const float LIFETIME = 6;

const int NPARTICLES = (int)(LIFETIME / PERIOD);

bool init()
{
	// switch on: transparency/blending

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// setup the point size
	glEnable(GL_POINT_SPRITE);
	glPointSize(50);

	// Initialise Shader - Basic
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!programBasic.create()) return false;
	if (!programBasic.attach(vertexShader)) return false;
	if (!programBasic.attach(fragmentShader)) return false;
	if (!programBasic.link()) return false;
	if (!programBasic.use(true)) return false;

	// Initialise Shader - Particle
	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/particles.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/particles.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!programParticle.create()) return false;
	if (!programParticle.attach(vertexShader)) return false;
	if (!programParticle.attach(fragmentShader)) return false;
	if (!programParticle.link()) return false;
	if (!programParticle.use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(programBasic.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(programBasic.getAttribLocation("aNormal"));

	// setup lights:

	// ambient
	programBasic.sendUniform("lightAmbient.color", vec3(0.1, 0.1, 0.1));

	// directional
	programBasic.sendUniform("lightDir.direction", vec3(1.0, 0.5, 1.0));
	programBasic.sendUniform("lightDir.diffuse", vec3(1.0, 1.0, 1.0));
	// Setup the particle system

	programParticle.sendUniform("initialPos", vec3(0.0, 0.58, 0.0));

	programParticle.sendUniform("gravity", vec3(0.0, 0.1, 0.0));

	programParticle.sendUniform("particleLifetime", LIFETIME);
	
	// Prepare the particle buffers

	std::vector<float> bufferVelocity;

	std::vector<float> bufferStartTime;

	float time = 0;

	for (int i = 0; i < NPARTICLES; i++)

	{

		float theta = (float)M_PI / 6.f * (float)rand() / (float)RAND_MAX;

		float phi = (float)M_PI * 2.f * (float)rand() / (float)RAND_MAX;

		float x = sin(theta) * cos(phi);

		float y = cos(theta);

		float z = sin(theta) * sin(phi);

		float v = 2 + 0.5f * (float)rand() / (float)RAND_MAX;


		bufferVelocity.push_back(x * v);

		bufferVelocity.push_back(y * v);

		bufferVelocity.push_back(z * v);


		bufferStartTime.push_back(time);

		time += PERIOD;

	}

	glGenBuffers(1, &idBufferVelocity);

	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0],

		GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferStartTime);

	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0],

		GL_STATIC_DRAW);
	// Setup the Texture
	C3dglBitmap bm("models/smoke.bmp", GL_RGBA);
    glGenTextures(1, &idTexParticle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, idTexParticle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());

	programParticle.sendUniform("texture0", 0);

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(-35.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(0.0, -0.5, 4.0),
		vec3(0.0, -0.5, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0, 0, 0, 1.0f);   // black background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;


	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;

	// teapot
	programBasic.use();
	programBasic.sendUniform("materialAmbient", vec3(0.8, 0.2, 0.2));
	programBasic.sendUniform("materialDiffuse", vec3(0.8, 0.2, 0.2));
	m = matrixView;
	m = translate(m, vec3(-0.77f, 0.38f, 0.0f));
	programBasic.sendUniform("matrixModelView", m);
	glutSolidTeapot(0.5);

	///////////////////////////////////
	// TO DO: RENDER THE PARTICLE SYSTEM
	// RENDER THE PARTICLE SYSTEM

	programParticle.use();


	m = matrixView;

	programParticle.sendUniform("matrixModelView", m);
	// particles
	glDepthMask(GL_FALSE);				// disable depth buffer updates
	glActiveTexture(GL_TEXTURE0);			// choose the active texture
	glBindTexture(GL_TEXTURE_2D, idTexParticle);	// bind the texture


	// render the buffer

	GLint aVelocity = programParticle.getAttribLocation("aVelocity");

	GLint aStartTime = programParticle.getAttribLocation("aStartTime");

	glEnableVertexAttribArray(aVelocity); // velocity

	glEnableVertexAttribArray(aStartTime); // start time

	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);

	glVertexAttribPointer(aVelocity, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);

	glVertexAttribPointer(aStartTime, 1, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_POINTS, 0, NPARTICLES);

	glDisableVertexAttribArray(aVelocity);

	glDisableVertexAttribArray(aStartTime);
	glDepthMask(GL_TRUE);		// don't forget to switch the depth test updates back on
}

void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime
	programParticle.sendUniform("time", time);
	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;

	// setup View Matrix
	programBasic.sendUniform("matrixView", matrixView);

	// render the scene objects
	renderScene(matrixView, time, deltaTime);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	
	mat4 m = perspective(radians(_fov), ratio, 0.02f, 1000.f);
	programBasic.sendUniform("matrixProjection", m);
	programParticle.sendUniform("matrixProjection", m);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)), 
		-pitch, vec3(1.f, 0.f, 0.f)) 
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: Particle System - Initial Configuration");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char *)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

