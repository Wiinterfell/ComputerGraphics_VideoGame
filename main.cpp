//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "maths_funcs.h"
#include <string>

#include "soil\src\SOIL.h"

// Assimp includes

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

#define CACTUS_NUMBER 20

float xTranslation = 0.0f;
float yTranslation = 0.0f;
float zTranslation = 0.0f;
float xRotation = 0.0f;
float yRotation = 0.0f;
float zRotation = 0.0f;

float rexX = -80.0f;
float rexY = 0.0f;
float rexZ = 0.0f;
float jumpSpeed = 0.0f;
float rexRotationX = 0.0f;
float rexRotationY = 0.0f;
float rexRotationZ = 0.0f;

float cactusX[CACTUS_NUMBER];
float cactusY[CACTUS_NUMBER];
float enemyX = 0.0f;
float enemyY = 0.0f;

unsigned char* imageCactus;
unsigned char* imageRex;
unsigned char* imageGround;
int widthCactus, heightCactus;
int widthRex, heightRex;
int widthGround, heightGround;

bool lost = false;

int addingScore = 0.0f;

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME "ground.3ds"
#define MESH_NAME_2 "d1.DAE"
#define MESH_NAME_3 "T_REX.3DS"
#define MESH_NAME_4 "cactus.3ds"
#define MESH_NAME_5 "star.3ds"

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

std::vector<float> g_vp, g_vn, g_vt;
int g_point_count = 0;
std::vector<int> g_point_counts;


// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
GLuint shaderProgramID;


unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;


#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

bool load_mesh(const char* file_name) {
	const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate); // TRIANGLES!
	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return false;
	}
	printf("  %i animations\n", scene->mNumAnimations);
	printf("  %i cameras\n", scene->mNumCameras);
	printf("  %i lights\n", scene->mNumLights);
	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	g_point_count = 0;
	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		g_point_count += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				//printf ("      vp %i (%f,%f,%f)\n", v_i, vp->x, vp->y, vp->z);
				g_vp.push_back(vp->x);
				g_vp.push_back(vp->y);
				g_vp.push_back(vp->z);
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				//printf ("      vn %i (%f,%f,%f)\n", v_i, vn->x, vn->y, vn->z);
				g_vn.push_back(vn->x);
				g_vn.push_back(vn->y);
				g_vn.push_back(vn->z);
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				//printf ("      vt %i (%f,%f)\n", v_i, vt->x, vt->y);
				g_vt.push_back(vt->x);
				g_vt.push_back(vt->y);
			}
			if (mesh->HasTangentsAndBitangents()) {
				// NB: could store/print tangents here
			}
		}
	}
	g_point_counts.push_back(g_point_count);
	aiReleaseImport(scene);
	return true;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile) {
	FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "../Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "../Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void loadTextures()
{
	GLuint tex; 
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	imageCactus = SOIL_load_image("cactus1.png", &widthCactus, &heightCactus, 0, SOIL_LOAD_RGB);
	imageRex = SOIL_load_image("trex.jpg", &widthRex, &heightRex, 0, SOIL_LOAD_RGB);
	imageGround = SOIL_load_image("desert.jpg", &widthGround, &heightGround, 0, SOIL_LOAD_RGB);

	GLint texAttrib = glGetAttribLocation(shaderProgramID, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(5 * sizeof(float)));
}

void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	load_mesh(MESH_NAME_2);
	load_mesh(MESH_NAME_3);
	load_mesh(MESH_NAME);
	load_mesh(MESH_NAME_4);
	load_mesh(MESH_NAME_5);
	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, (g_point_counts[0] + g_point_counts[1] + g_point_counts[2] + g_point_counts[3] + g_point_counts[4]) * 3 * sizeof(float), &g_vp[0], GL_STATIC_DRAW);
	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, (g_point_counts[0] + g_point_counts[1] + g_point_counts[2] + g_point_counts[3] + g_point_counts[4]) * 3 * sizeof(float), &g_vn[0], GL_STATIC_DRAW);


	unsigned int vt_vbo = 0;

	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	loadTextures();
}


#pragma endregion VBO_FUNCTIONS

void print(int x, int y, int z, const char *string)
{
	//set the position of the text in the window using the x and y coordinates
	glRasterPos3f(x, y, z);
	//get the length of the string to display
	int len = (int)strlen(string);

	//loop to display character by character
	for (int i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
};

void display() {

	if (lost)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgramID);
		string score = to_string((int)(rexX * 10 + 1000 + addingScore));
		string scoreStr = "SCORE: " + score;
		const char * scoreChar = scoreStr.c_str();
		print(0.0f, 0.0f, 0.0f, scoreChar);
		glutSwapBuffers();
		return;
	}


	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.0f, 0.7f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	//Light
	int light_pos = glGetUniformLocation(shaderProgramID, "light_pos");
	glUniform3f(light_pos, rexX, rexY, rexZ + 5.0);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");

	//Camera - ground
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthGround, heightGround, 0, GL_RGB, GL_UNSIGNED_BYTE, imageGround);

	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 100.0);
	mat4 model = identity_mat4();

	model = scale(model, vec3(100.0f, 0.05f, 1.0f));
	model = translate(model, vec3(0.0f, 0.0f, 3.7f));
	view = look_at(vec3(rexX - 1.0f, 0.0f + rexY, 0.5f + rexZ), vec3(rexX, rexY, rexZ + 0.5f), vec3(0.0f, 0.0f, 1.0f));
	view = translate(view, vec3(xTranslation, yTranslation, zTranslation));
	view = rotate_x_deg(view, xRotation);
	view = rotate_y_deg(view, yRotation);
	view = rotate_z_deg(view, zRotation);
	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);

	glDrawArrays(GL_TRIANGLES, g_point_counts[0] + g_point_counts[1], g_point_counts[2]);
	


	//cactus obstacle
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthCactus, heightCactus, 0, GL_RGB, GL_UNSIGNED_BYTE, imageCactus);
	
	for (int i = 0; i < CACTUS_NUMBER; i++)
	{
		mat4 model3 = identity_mat4();
		model3 = scale(model3, vec3(0.009f, 0.009f, 0.009f));
		model3 = translate(model3, vec3(cactusX[i], cactusY[i], 0.0f));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model3.m);
		glDrawArrays(GL_TRIANGLES, g_point_counts[0] + g_point_counts[1] + g_point_counts[2], g_point_counts[3]);

		//star - hierarchy for cactus
		model3 = translate(model3, vec3(-cactusX[i], -cactusY[i], 0.0f));
		model3 = scale(model3, vec3(4.0f, 4.0f, 4.0f));
		model3 = rotate_y_deg(model3, 90.0f);
		model3 = rotate_z_deg(model3, glutGet(GLUT_ELAPSED_TIME));
		model3 = translate(model3, vec3(cactusX[i], cactusY[i], 0.0f));
		model3 = translate(model3, vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model3.m);
		glDrawArrays(GL_TRIANGLES, g_point_counts[0] + g_point_counts[1] + g_point_counts[2] + g_point_counts[3], g_point_counts[4]);
	}




	//up obstacle
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthRex, heightRex, 0, GL_RGB, GL_UNSIGNED_BYTE, imageRex);
	mat4 model4 = identity_mat4();
	model4 = scale(model4, vec3(0.009f, 0.009f, 0.009f));
	model4 = rotate_z_deg(model4, 90.0f);
	model4 = translate(model4, vec3(enemyX, enemyY, 1.5f));
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model4.m);
	glDrawArrays(GL_TRIANGLES, 0, g_point_counts[0]);


	//dinosaur
	mat4 model5 = identity_mat4();
	model5 = translate(model5, vec3(0.0, -5.0, 0.0));
	model5 = scale(model5, vec3(0.1f, 0.1f, 0.1f));
	model5 = rotate_z_deg(model5, 90.0f);
	model5 = translate(model5, vec3(rexX, rexY, rexZ));
	model5 = rotate_z_deg(model5, rexRotationZ);
	model5 = rotate_y_deg(model5, rexRotationY);
	model5 = rotate_x_deg(model5, rexRotationX);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model5.m);
	glDrawArrays(GL_TRIANGLES, g_point_counts[0], g_point_counts[1]);

	glutSwapBuffers();

}

void generateObstacles()
{
	for (int i = 0; i < CACTUS_NUMBER; i++)
	{
		if (cactusX[i] == 0.0f && cactusY[i] == 0.0f)
		{
			int randY = rand() % (3 + 3 + 1) - 3;
			cactusX[i] = rexX + 50.0f + (i * 10.0f / 5);
			cactusY[i] = (float) randY;
		}
		else if (cactusX[i] <= rexX + 0.5f)
		{
			cactusX[i] = 0.0f;
			cactusY[i] = 0.0f;
		}
	}


	if (enemyX == 0.0f && enemyY == 0.0f)
	{
		int randEnemy = rand() % (10 + 1);
		enemyX = rexX + 60.0f + randEnemy;
		enemyY = 0.0f;
	}
	else if (enemyX <= rexX)
	{
		enemyX = 0.0f;
		enemyY = 0.0f;
	}
}

bool collision()
{
	for (int i = 0; i < CACTUS_NUMBER; i++)
	{
		bool x = (cactusX[i] - rexX) < 1.3f && (cactusX[i] - rexX) > 0.9f;
		bool y = abs(rexY - cactusY[i]) < 0.1f;
		bool z = rexZ < 1.0f;
		if (x && y && z)
		{
			return true;
		}
		else if (x && y)
		{
			addingScore += 50.0f;
			cout << addingScore << endl;
		}
	}
	bool x = abs(rexX - enemyX) < 1.3f;
	bool z = rexZ > 1.0f;
	if (x && z)
	{
		return true;
	}

	return false;
}

void updateScene() {

	if (lost)
	{
		return;
	}

	//the dinosaur is moving
	rexX += 0.03;
	rexZ += jumpSpeed;
	if (rexZ > 0)
	{
		jumpSpeed -= 0.0004f;
	}
	else
	{
		jumpSpeed = 0.0f;
	}

	//you fall
	if (rexY >= 4.0f || rexY <= -4.0f)
	{
		rexZ -= 0.1f;
	}
	//don't miss a frame on and of jump
	else if (rexZ <= 0.0f)
	{
		rexZ = 0.0f;
	}

	//score points
	int score = rexX * 10 + 1000;
	score += addingScore;
	if (score % 1000 == 0)
	{
		PlaySound("points.wav", NULL, SND_ASYNC | SND_FILENAME);
	}

	//game lost
	if (rexZ <= -5.0f || collision())
	{
		PlaySound("dead.wav", NULL, SND_ASYNC | SND_FILENAME);
		lost = true;
		cout << "LOST" << endl;
	}

	generateObstacles();

	// Draw the next frame
	glutPostRedisplay();
}

void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();

	for (int i = 0; i < CACTUS_NUMBER; i++)
	{
		cactusX[i] = 0.0f;
		cactusY[i] = 0.0f;
	}

	// load mesh into a vertex buffer array
	generateObjectBufferMesh();

}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	switch (key) {
		//----------- REINITIALISATION -----------//
	case 'w':
		xTranslation = 0.0f;
		yTranslation = 0.0f;
		zTranslation = 0.0f;
		xRotation = 0.0f;
		yRotation = 0.0f;
		zRotation = 0.0f;
		break;
		//----------- TRANSLATION -----------//
	case 'z':
		xTranslation -= 0.2f * sin(yRotation * 3.14f / 180);
		zTranslation += 0.2f * cos(yRotation * 3.14f / 180);
		break;
	case 's':
		xTranslation += 0.2f * sin(yRotation * 3.14f / 180);
		zTranslation -= 0.2f * cos(yRotation * 3.14f / 180);
		break;
		//----------- POSITIVE ROTATION -----------//
	case '1':
		xRotation += 0.6f;
		break;
	case 'd':
		yRotation += 0.6f;
		break;
	case '2':
		zRotation += 0.6f;
		break;
		//----------- NEGATIVE ROTATION -----------//
	case '3':
		xRotation -= 0.6f;
		break;
	case 'q':
		yRotation -= 0.6f;
		break;
	case '4':
		zRotation -= 0.6f;
		break;
		//----------- T REX MOTION -----------//
	case '+':
		rexX += 0.1f;
		break;
	case '-':
		rexX -= 0.1f;
		break;
	case ' ':
		if (jumpSpeed == 0.0f)
		{
			jumpSpeed = 0.04f;
			PlaySound("jump.wav", NULL, SND_ASYNC | SND_FILENAME);
		}
		if (lost)
		{
			lost = false;
			addingScore = 0.0f;
			xTranslation = 0.0f;
			yTranslation = 0.0f;
			zTranslation = 0.0f;
			xRotation = 0.0f;
			yRotation = 0.0f;
			zRotation = 0.0f;
			rexX = -80.0f;
			rexY = 0.0f;
			rexZ = 0.0f;
			jumpSpeed = 0.0f;
			rexRotationX = 0.0f;
			rexRotationY = 0.0f;
			rexRotationZ = 0.0f;
			for (int i = 0; i < CACTUS_NUMBER; i++)
			{
				cactusX[i] = 0.0f;
				cactusY[i] = 0.0f;
			}
			enemyX = 0.0f;
			enemyY = 0.0f;
		}
		break;
	case 'c':
		rexY += 1.0f;
		break;
	case 'v':
		rexY -= 1.0f;
		break;
	}
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Offline game");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}











