#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "main.h"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

float vertices[] = {

-4, 0, -4, 4, 0, -4, 0, 7, 0, -4, 0, 4, 4, 0, 4, 0, 7, 0,

-4, 0, -4, -4, 0, 4, 0, 7, 0, 4, 0, -4, 4, 0, 4, 0, 7, 0,

-4, 0, -4, -4, 0, 4, 4, 0, -4, 4, 0, 4 };

float normals[] = {

0, 4, -7, 0, 4, -7, 0, 4, -7, 0, 4, 7, 0, 4, 7, 0, 4, 7,

-7, 4, 0, -7, 4, 0, -7, 4, 0, 7, 4, 0, 7, 4, 0, 7, 4, 0,

0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0 };

unsigned indices[] = {

0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 14, 15 };


// buffers names

unsigned vertexBuffer = 0;

unsigned normalBuffer = 0;

unsigned indexBuffer = 0;	




// 3D models
C3dglModel table;
C3dglModel chair;
C3dglModel teapot;
C3dglModel Vase;
C3dglModel Figure; 
C3dglModel lamp;	
C3dglModel knight;
//animations
C3dglModel dance;
// textures
C3dglBitmap bm;
C3dglBitmap oak;
GLuint idTexWood;
GLuint idTexCloth;
GLuint idTexNone;
GLuint idTexCube;
GLuint idTexShadowMap;

// frame buffer objects
GLuint idFBO;
// The View Matrix
mat4 matrixView;
// GLSL programs
C3dglProgram program;
// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)
bool isLamp1on = false;	// light on/off switchc
bool isLamp2on = false;	// light on/off switch
bool init()
{
	
	C3dglShader vertexShader;

	C3dglShader fragmentShader;
	

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;

	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;

	if (!vertexShader.compile()) return false;


	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;

	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;

	if (!fragmentShader.compile()) return false;


	if (!program.create()) return false;

	if (!program.attach(vertexShader)) return false;

	if (!program.attach(fragmentShader)) return false;

	if (!program.link()) return false;

	if (!program.use(true)) return false;
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// prepare vertex data

	glGenBuffers(1, &vertexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	// prepare normal data

	glGenBuffers(1, &normalBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);


	// prepare indices array

	glGenBuffers(1, &indexBuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);            

	

	// load your 3D models here!
	if (!table.load("models\\table.obj")) return false;
	if (!chair.load("models\\table.obj")) return false;
	if (!teapot.load("models\\utah_teapot_ultrares.obj")) return false;
	if (!Vase.load("models\\vase.obj")) return false;
	if (!Figure.load("models\\figure.fbx")) return false;
	if (!lamp.load("models\\lamp.3ds")) return false;
	//animations
	dance.load("models\\dance.fbx");
	knight.load("models\\knight.fbx");
	knight.loadMaterials("models\\knight.fbx");
	knight.loadAnimations(&dance);
	// load your textures here!
	oak.load("models/oak.bmp", GL_RGBA);
	if (!oak.getBits()) return false;
	bm.load("models/cloth.bmp", GL_RGBA);
	if (!bm.getBits()) return false;
	// prepare the texture oak 
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, oak.getWidth(), oak.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, oak.getBits());
	// prepare the texture cloth
	glGenTextures(1, &idTexCloth);
	glBindTexture(GL_TEXTURE_2D, idTexCloth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());

	//prepare the null texture
	// none (simple-white) texture

	glGenTextures(1, &idTexNone);

	glBindTexture(GL_TEXTURE_2D, idTexNone);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	BYTE bytes[] = { 255, 255, 255 };

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);
	// load Cube Map

	glActiveTexture(GL_TEXTURE1);

	glGenTextures(1, &idTexCube);

	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// Send the texture info to the shaders
	program.sendUniform("textureCubeMap", 1);
	program.sendUniform("texture0", 0);
	// Create shadow map texture

	glActiveTexture(GL_TEXTURE7);

	glGenTextures(1, &idTexShadowMap);

	glBindTexture(GL_TEXTURE_2D, idTexShadowMap);


	// Texture parameters - to get nice filtering & avoid artefact on the edges of the shadowmap

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);


	// This will associate the texture with the depth component in the Z-buffer

	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);

	int w = viewport[2], h = viewport[3];

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w * 2, h * 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);


	// Send the texture info to the shaders

	program.sendUniform("shadowMap", 7);
	// Create a framebuffer object (FBO)
	
	glGenFramebuffers(1, &idFBO);

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, idFBO);


	// Instruct openGL that we won't bind a color texture with the currently binded FBO

	glDrawBuffer(GL_NONE);

	glReadBuffer(GL_NONE);


	// attach the texture to FBO depth attachment point

	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, idTexShadowMap, 0);


	// switch back to window-system-provided framebuffer

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	// revert to texture unit 0

	glActiveTexture(GL_TEXTURE0);
	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(0.0, 5.0, 10.0),
		vec3(0.0, 5.0, 0.0),
		vec3(0.0, 1.0, 0.0));
	program.sendUniform("matrixView", matrixView);
	// setup the screen background colour
	glClearColor(0.18f, 0.25f, 0.22f, 1.0f);   // deep grey background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift to speed up your movement" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;
	
	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));

	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));
	
	return true;
	
	
}
void renderVase(mat4 matrixView, float time, float deltaTime)
{
	mat4 m;	
	// set up materials - red 
	program.sendUniform("materialAmbient", vec3(0.6f, 0.1f, 0.1f));
	program.sendUniform("materialDiffuse", vec3(0.6f, 0.1f, 0.1f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("shininess", 100.0f);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	// vase
	m = matrixView;
	m = translate(m, vec3(-2.0f, 3.0421f, 0.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));

	Vase.render(m);
}
void renderPyramid(mat4 matrixView, float time, float deltaTime)
{
	// Set up materials for the pyramid - green
	program.sendUniform("materialAmbient", vec3(0.1f, 0.6f, 0.1f));
	program.sendUniform("materialDiffuse", vec3(0.1f, 0.6f, 0.1f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("shininess", 100.0f);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	// Get Attribute Locations
	GLuint attribVertex = program.getAttribLocation("aVertex");
	GLuint attribNormal = program.getAttribLocation("aNormal");

	// Enable vertex attribute arrays
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);

	// Bind (activate) the vertex buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind (activate) the normal buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Apply scaling transformation to the pyramid
	mat4 m = matrixView;
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	m = translate(m, vec3(0.0f, 67.84f, 0.0f));
	m = rotate(m, radians(180.0f), vec3(0.0f, 0.0f, 1.0f));
	m = rotate(m, time, vec3(0.0f, 1.0f, 0.0f));

	// Send the model-view matrix to the shader
	program.sendUniform("matrixModelView", m);

	// Draw triangles – using index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	// Disable arrays
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribNormal);
}


void renderScene(mat4& matrixView, float time, float deltaTime)
{



	mat4 m;

	
	// ambient lighting 
	program.sendUniform("lightAmbient1.color", vec3(0.1, 0.1, 0.1));
	
	// directional light
	program.sendUniform("lightDir.direction", vec3(1.0, 0.5, 1.0));
	program.sendUniform("lightDir.diffuse", vec3(0.7, 0.7, 0.7));

	// point light (not specular) 1
	if (isLamp1on == true)
	{

	program.sendUniform("lightPoint1.diffuse", vec3(1.0, 1.0, 1.0));
	
	program.sendUniform("lightPoint1.position", vec3(1.0f, 4.95f, 0.0f));
	}
	else
	{
		program.sendUniform("lightPoint1.diffuse", vec3(0.0, 0.0, 0.0));
		
		

    }
	program.sendUniform("isLamp1on", isLamp1on);
	

	// point light 2 (specular)
	if (isLamp2on == true)
	{
		program.sendUniform("lightPoint2.diffuse", vec3(1.0, 1.0, 1.0));
		program.sendUniform("lightPoint2.specular", vec3(1.0, 1.0, 1.0));
		program.sendUniform("lightPoint2.position", vec3(-1.0f, 4.95f, 0.0f));
	}
	else
	{
		program.sendUniform("lightPoint2.diffuse", vec3(0.0, 0.0, 0.0));
		program.sendUniform("lightPoint2.specular", vec3(0.0, 0.0, 0.0));
		
	}
	
	program.sendUniform("isLamp2on", isLamp2on);
	// set up materials green
	program.sendUniform("materialAmbient", vec3(0.1f, 0.6f, 0.1f));
	program.sendUniform("materialDiffuse", vec3(0.1f, 0.6f, 0.1f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("shininess", 100.0f);
	

	// set up materials brown
	program.sendUniform("materialAmbient", vec3(0.6f, 0.3f, 0.1f));
	program.sendUniform("materialDiffuse", vec3(0.6f, 0.3f, 0.1f));
	program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
	program.sendUniform("shininess", 1.0f);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	
	// table
	m = matrixView;
	m = translate(m, vec3(0.0f, 0, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	
	table.render(1, m);


	// set up materials white 
	program.sendUniform("materialAmbient", vec3(0.6f, 0.6f, 0.6f));
	program.sendUniform("materialDiffuse", vec3(0.6f, 0.6f, 0.6f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("shininess", 100.0f);
	glBindTexture(GL_TEXTURE_2D, idTexCloth);
	// chair 1
	m = matrixView;
	m = translate(m, vec3(0.0f, 0, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));

	table.render(0, m);



	// chair 2
	m = matrixView;
	m = translate(m, vec3(0.0f, 0, 0.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	
	chair.render(0, m);
	// chair 3
	m = matrixView;
	m = translate(m, vec3(0.0f, 0, 0.0f));
	m = rotate(m, radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	
	chair.render(0, m);
	// chair 4
	m = matrixView;
	m = translate(m, vec3(0.0f, 0, 0.0f));
	m = rotate(m, radians(270.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	
	chair.render(0, m);
	// set up materials - blue
	program.sendUniform("materialAmbient", vec3(0.1f, 0.1f, 0.6f));
	program.sendUniform("materialDiffuse", vec3(0.1f, 0.1f, 0.6f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("shininess", 10.0f);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	// teapot
	m = matrixView;
	m = translate(m, vec3(2.0f, 3.029f, 0.0f));
	m = rotate(m, radians(120.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.5f, 0.5f, 0.5f));
	program.sendUniform("matrixView", matrixView);
	teapot.render(m);
	// set up materials - red 
	program.sendUniform("materialAmbient", vec3(0.6f, 0.1f, 0.1f));
	program.sendUniform("materialDiffuse", vec3(0.6f, 0.1f, 0.1f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("shininess", 100.0f);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	
	// figure 
	m = matrixView;
	m = translate(m, vec3(0.0f,3.391f, 0.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	program.sendUniform("matrixView", matrixView);
	
	m = rotate(m, time, vec3(0.0f, 1.0f, 0.0f));
	Figure.render(m);
	//set up materials yellow 
	program.sendUniform("materialAmbient", vec3(0.6f, 0.6f, 0.1f));
	program.sendUniform("materialDiffuse", vec3(0.6f, 0.6f, 0.1f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("shininess", 100.0f);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	// lamp

	m = matrixView;
	m = translate(m, vec3(1.0f, 6.0f, 0.0f));
	m = rotate(m, radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));

	lamp.render(m);
	
	//lamp 2
	m = matrixView;
	m = translate(m, vec3(-1.0f, 6.0f, 0.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));
	
	lamp.render(m);
	
	// set up materials white bulb 1
	if (isLamp1on == true)

	{
		program.sendUniform("materialAmbient", vec3(1.0f, 1.0f, 1.0f));
		program.sendUniform("materialDiffuse", vec3(0.0f, 0.0f, 0.0f));
		program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
		program.sendUniform("shininess", 100.0f);
		program.sendUniform("lightAmbient2.color", vec3(1.0, 1.0, 1.0));
		glBindTexture(GL_TEXTURE_2D, idTexNone);
	}
	else
	{
		program.sendUniform("materialAmbient", vec3(0.5f, 0.5f, 0.5f));
		program.sendUniform("materialDiffuse", vec3(0.0f, 0.0f, 0.0f));
		program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
		program.sendUniform("shininess", 100.0f);
		program.sendUniform("lightAmbient2.color", vec3(0.0, 0.0, 0.0));
		glBindTexture(GL_TEXTURE_2D, idTexNone);

	}
	// light bulb 1
	m = matrixView;
	m = translate(m, vec3(1.0f, 4.95f, 0.0f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	program.sendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	if (isLamp2on)
	{
		program.sendUniform("materialAmbient", vec3(1.0f, 1.0f, 1.0f));
		program.sendUniform("materialDiffuse", vec3(0.0f, 0.0f, 0.0f));
		program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
		program.sendUniform("shininess", 100.0f);
		program.sendUniform("lightAmbient2.color", vec3(1.0, 1.0, 1.0));
		glBindTexture(GL_TEXTURE_2D, idTexNone);

	}
	else
	{
		program.sendUniform("materialAmbient", vec3(0.5f, 0.5f, 0.5f));
		program.sendUniform("materialDiffuse", vec3(0.0f, 0.0f, 0.0f));
		program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
		program.sendUniform("shininess", 100.0f);
		program.sendUniform("lightAmbient2.color", vec3(0.0, 0.0, 0.0));
		glBindTexture(GL_TEXTURE_2D, idTexNone);
	}


	// light bulb 2
	m = matrixView;
	m = translate(m, vec3(-1.0f, 4.95f, 0.0f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	program.sendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	program.sendUniform("lightAmbient2.color", vec3(0.0, 0.0, 0.0));
	// calculate and send bone transforms
	std::vector<mat4> transforms;
	knight.getAnimData(0, time, transforms);
	program.sendUniform("bones", &transforms[0], transforms.size());

	m = matrixView;
	m = translate(m, vec3(1.0f, 3.035f, 0.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.001f, 0.001f, 0.001f));
	knight.render(m);


}
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	program.sendUniform("matrixProjection", matrixProjection);
}

void prepareCubeMap(float x, float y, float z, float time, float deltaTime) {
	//Modified version of the one given in the recipe because the one in the recipe had an issue where the viewport wasn't restoring properply 
	// Store the complete current viewport state
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Store these for later use
	int originalX = viewport[0];
	int originalY = viewport[1];
	int originalWidth = viewport[2];
	int originalHeight = viewport[3];

	// setup the viewport to 256x256, 90 degrees FoV
	glViewport(0, 0, 256, 256);
	program.sendUniform("matrixProjection", perspective(radians(90.f), 1.0f, 0.02f, 1000.0f));

	// Make sure we're in modelview mode
	glMatrixMode(GL_MODELVIEW);

	// render environment 6 times
	program.sendUniform("reflectionPower", 0.0);

	for (int i = 0; i < 6; ++i) {
		// clear background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// setup the camera
		const GLfloat ROTATION[6][6] =
		{ // at up
			{ 1.0, 0.0, 0.0, 0.0, -1.0, 0.0 }, // pos x
			{ -1.0, 0.0, 0.0, 0.0, -1.0, 0.0 }, // neg x
			{ 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 }, // pos y
			{ 0.0, -1.0, 0.0, 0.0, 0.0, -1.0 }, // neg y
			{ 0.0, 0.0, 1.0, 0.0, -1.0, 0.0 }, // pos z
			{ 0.0, 0.0, -1.0, 0.0, -1.0, 0.0 } // neg z
		};

		mat4 matrixView2 = lookAt(
			vec3(x, y, z),
			vec3(x + ROTATION[i][0], y + ROTATION[i][1], z + ROTATION[i][2]),
			vec3(ROTATION[i][3], ROTATION[i][4], ROTATION[i][5])
		);

		// send the View Matrix
		program.sendUniform("matrixView", matrixView2);
		
		// render scene objects - all but the reflective one
		glActiveTexture(GL_TEXTURE0);
		renderScene(matrixView2, time, deltaTime);
		renderPyramid(matrixView2, time, deltaTime);

		// send the image to the cube texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, 0, 0, 256, 256, 0);
	}

	// Explicitly restore the original viewport state
	glViewport(originalX, originalY, originalWidth, originalHeight);

	// Restore the projection matrix through onReshape
	onReshape(originalWidth, originalHeight);

	// Restore the original view matrix
	program.sendUniform("matrixView", matrixView);
	
}

void createShadowMap(mat4 lightTransform, float time, float deltaTime)

{
	glEnable(GL_CULL_FACE);
	

	glCullFace(GL_FRONT);


	// Store the current viewport in a safe place

	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);

	int w = viewport[2], h = viewport[3];

	

	// setup the viewport to 2x2 the original and wide (120 degrees) FoV (Field of View)

	glViewport(0, 0, w * 2, h * 2);

	mat4 matrixProjection = perspective(radians(160.f), (float)w / (float)h, 0.5f, 50.0f);

	program.sendUniform("matrixProjection", matrixProjection);


	// prepare the camera

	mat4 matrixView = lightTransform;


	// send the View Matrix

	program.sendUniform("matrixView", matrixView);


	// Bind the Framebuffer

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, idFBO);

	// OFF-SCREEN RENDERING FROM NOW!


	// Clear previous frame values - depth buffer only!

	glClear(GL_DEPTH_BUFFER_BIT);


	// Disable color rendering, we only want to write to the Z-Buffer (this is to speed-up)

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);


	// Prepare and send the Shadow Matrix - this is matrix transform every coordinate x,y,z

	// x = x* 0.5 + 0.5

	// y = y* 0.5 + 0.5

	// z = z* 0.5 + 0.5

	// Moving from unit cube [-1,1] to [0,1]

	const mat4 bias = {

	{ 0.5, 0.0, 0.0, 0.0 },

	{ 0.0, 0.5, 0.0, 0.0 },

	{ 0.0, 0.0, 0.5, 0.0 },

	{ 0.5, 0.5, 0.5, 1.0 }

	};

	program.sendUniform("matrixShadow", bias * matrixProjection * matrixView);


	// Render all objects in the scene

	renderScene(matrixView, time, deltaTime);
	renderVase(matrixView, time, deltaTime);
	glDisable(GL_CULL_FACE);
	renderPyramid(matrixView, time,deltaTime);
	glEnable(GL_CULL_FACE);
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	

	
	
	onReshape(w, h);

	
}
void clearShadowMap()
{
	// used when the lights are off
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, idFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime
	
	if (isLamp2on || isLamp1on)
	{
		if (isLamp2on && !isLamp1on)
		{
			createShadowMap(lookAt(
				vec3(-1.0f, 4.95f, 0.0f), // coordinates of the source of the light
				vec3(-1.0f, 0.0f, 0.0f),  // directly below the light source
				vec3(0.0f, 0.0f, -1.0f)), // a reasonable "Up" vector
				time, deltaTime);
		}

		if (isLamp1on && !isLamp2on)
		{
			createShadowMap(lookAt(
				vec3(1.0f, 4.95f, 0.0f), // coordinates of the source of the light
				vec3(1.0f, 0.0f, 0.0f),  // directly below the light source
				vec3(0.0f, 0.0f, -1.0f)), // a reasonable "Up" vector
				time, deltaTime);
		}

		// if they're both on create a map in the middle of the two 
		if (isLamp1on && isLamp2on)
		{
			createShadowMap(lookAt(
				vec3(0.01f, 4.95f, 0.0f), // coordinates of the source of the light
				vec3(0.01f, 0.0f, 0.0f),  // directly below the light source
				vec3(0.0f, 0.0f, -1.0f)), // a reasonable "Up" vector
				time, deltaTime);
		}
	}
	
	
	else
	{
		clearShadowMap();
		
	}
	
	glDisable(GL_CULL_FACE);
	//prepare cubemap
	prepareCubeMap(-2.0f, 3.5021f, 0.0f, time, deltaTime);
	glActiveTexture(GL_TEXTURE0);
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

	// render the scene objects

	

	program.sendUniform("reflectionPower", 0.0);
	renderPyramid(matrixView, time, deltaTime);
	renderScene(matrixView, time, deltaTime);
	
	
	glActiveTexture(GL_TEXTURE1);
	program.sendUniform("reflectionPower", 0.5);
	renderVase(matrixView, time, deltaTime);
	
	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
	
}

// called before window opened or resized - to setup the Projection Matrix


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
	case '1': isLamp1on = !isLamp1on; break;
	case '2': isLamp2on = !isLamp2on; break;
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
	glutCreateWindow("3DGL Scene: First Example");

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

