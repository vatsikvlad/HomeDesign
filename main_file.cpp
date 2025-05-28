/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

double speed_x=0; //angular speed in radians
double speed_y=0; //angular speed in radians
double aspectRatio=1;
double camera_distance = 3.0f;
bool right_button_pressed = false;
double last_x = 0, last_y = 0;
ShaderProgram *sp; //Pointer to the shader program
GLuint tex0;
GLuint tex1;

//Uncomment to draw a teapot
float* vertices = myTeapotVertices;
float* texCoords = myTeapotTexCoords;
float* colors = myTeapotColors;
float* normals = myTeapotVertexNormals;
int vertexCount = myTeapotVertexCount;

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	//global declaration
	 //Read into computers memory
	std::vector<unsigned char> image;   //Allocate memory 
	unsigned width, height;   //Variables for image size
	//Read the image
	unsigned error = lodepng::decode(image, width, height, filename);
	//Import to graphics card memory
	glGenTextures(1, &tex); //Initialize one handle
	glBindTexture(GL_TEXTURE_2D, tex); //Activate handle
	//Copy image to graphics cards memory reprezented by the active handle
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return tex;
}

//Error processing callback procedure
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {	
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			right_button_pressed = true;
			glfwGetCursorPos(window, &last_x, &last_y);
		}
		else if (action == GLFW_RELEASE) {
			right_button_pressed = false;
		}
	}
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	if (right_button_pressed) {
		double dx = xpos - last_x;
		double dy = ypos - last_y;
		last_x = xpos;
		last_y = ypos;

		speed_x = dx * 1.0f;	// sensitivity myszy po osi x
		speed_y = -dy * 1.0f;	// sensitivity myszy po osi y (inverted)
	}
	else {
		speed_x = 0;
		speed_y = 0;
	}
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	camera_distance -= yoffset * 1.0f;
	if (camera_distance < 1.0f) camera_distance = 0.5f; // scroll sensitivity
	if (camera_distance > 10.0f) camera_distance = 10.0f;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action==GLFW_PRESS) {
        if (key==GLFW_KEY_LEFT) speed_x=-PI/2;
        if (key==GLFW_KEY_RIGHT) speed_x=PI/2;
        if (key==GLFW_KEY_UP) speed_y=PI/2;
        if (key==GLFW_KEY_DOWN) speed_y=-PI/2;
    }
    if (action==GLFW_RELEASE) {
        if (key==GLFW_KEY_LEFT) speed_x=0;
        if (key==GLFW_KEY_RIGHT) speed_x=0;
        if (key==GLFW_KEY_UP) speed_y=0;
        if (key==GLFW_KEY_DOWN) speed_y=0;
    }
}

void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(double)width/(double)height;
    glViewport(0,0,width,height);
}


void loadModel(std::string plik) {
	using namespace std;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	cout << importer.GetErrorString() << endl;
	
	//scene->

	//if (scene->HasMeshes()) {//Czy w pliku są siatki wielokątów
	//	for (int i = 0; i < scene->mNumMeshes; i++) { //Liczba siatek wielokątów
	//		importujMesh(scene->mMeshes[i]); //Dostęp do konkretnej siatki
	//	}
	//}

	aiMesh* mesh = scene->mMeshes[0];
	for (int i = 0; i < mesh->mNumVertices; i++) {
		aiVector3D vertex = mesh->mVertices[i]; //aiVector3D podobny do glm::vec3
		cout << vertex.x << " " << vertex.y << " " << vertex.z << endl;
		aiVector3D normal = mesh->mNormals[i]; //Wektory znormalizowane
		cout << normal.x << " " << normal.y << " " << normal.z << endl;
		unsigned int liczba_zest = mesh->GetNumUVChannels();	//liczba zdefiniowanych zestawów wsp. teksturowania (zestawów jest max 8)
		unsigned int wymiar_wsp_tex = mesh->mNumUVComponents[0];	//Liczba składowych wsp. teksturowania dla 0 zestawu.
		aiVector3D texCoord = mesh->mTextureCoords[0][i];	//0 to numer zestawu współrzędnych teksturowania	
		cout << texCoord.x << " " << texCoord.y << endl;	//x,y,z wykorzystywane jako u,v,w. 0 jeżeli tekstura ma mniej wymiarów
	}

	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace& face = mesh->mFaces[i]; //face to jeden z wielokątów siatki

		//dla każdego indeksu->wierzchołka tworzącego wielokąt
		//dla aiProcess_Triangulate to zawsze będzie 3
		for (int j = 0; j < face.mNumIndices; j++) {
			cout << face.mIndices[j] << " ";
		}
		cout << endl;
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	for (int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
		aiString str;//nazwa pliku
		aiTextureMapping mapping;//jak wygenerowano wsp. teksturowania (opcj.)
		unsigned int uvMapping;// numer zestawu wsp. teksturowania (opcjonalne)
		ai_real blend; // współczynnik połączenia kolorów z kolejną teksturą (op.)
		aiTextureOp op; //sposób łączenia kolorów z kolejną teksturą (opcjonalne)
		aiTextureMapMode mapMode; //sposób adresowania tekstury (opcjonalne)
		material->GetTexture(aiTextureType_DIFFUSE, i, &str, &mapping, &uvMapping, &blend, &op, &mapMode);
	}
}

//Initialization code procedure
void initOpenGLProgram(GLFWwindow* window) {
	//************Place any code here that needs to be executed once, at the program start************
	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);

	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetScrollCallback(window, scrollCallback);

	sp=new ShaderProgram("v_simplest.glsl",NULL,"f_simplest.glsl");
	tex0 = readTexture("metal.png");
	tex1 = readTexture("sky.png");
}

//Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow* window) {
	//************Place any code here that needs to be executed once, after the main loop ends************

	delete sp;
}

//Drawing procedure
void drawScene(GLFWwindow* window,float angle_x,float angle_y) {
	//************Place any code here that draws something inside the window******************l

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V=glm::lookAt(
        glm::vec3(0.0f,0.0f,-camera_distance),
        glm::vec3(0.0f,0.0f,0.0f),
        glm::vec3(0.0f,1.0f,0.0f)); //compute view matrix
    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, 1.0f, 1.0f, 50.0f); //compute projection matrix

    sp->use();//activate shading program
    //Send parameters to graphics card
    glUniformMatrix4fv(sp->u("P"),1,false,glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"),1,false,glm::value_ptr(V));

    glm::mat4 M=glm::mat4(1.0f);
	M=glm::rotate(M,angle_y,glm::vec3(1.0f,0.0f,0.0f)); //Compute model matrix
	M=glm::rotate(M,angle_x,glm::vec3(0.0f,1.0f,0.0f)); //Compute model matrix
    glUniformMatrix4fv(sp->u("M"),1,false,glm::value_ptr(M));


    glEnableVertexAttribArray(sp->a("vertex")); //Enable sending data to the attribute vertex
    glVertexAttribPointer(sp->a("vertex"),4,GL_FLOAT,false,0, vertices); //Specify source of the data for the attribute vertex

	glEnableVertexAttribArray(sp->a("color")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, colors); //Specify source of the data for the attribute color

	glEnableVertexAttribArray(sp->a("normal")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normals); //Specify source of the data for the attribute normal

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, tex0);
	glUniform1i(sp->u("textureMap0"), 10);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glUniform1i(sp->u("textureMap1"), 6);

	glEnableVertexAttribArray(sp->a("texCoord0")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords); //Specify source of the data for the attribute normal

    glDrawArrays(GL_TRIANGLES,0, vertexCount); //Draw the object

    glDisableVertexAttribArray(sp->a("vertex")); //Disable sending data to the attribute vertex
	glDisableVertexAttribArray(sp->a("color")); //Disable sending data to the attribute color
	glDisableVertexAttribArray(sp->a("normal")); //Disable sending data to the attribute normal
	glDisableVertexAttribArray(sp->a("texCoord0"));

    glfwSwapBuffers(window); //Copy back buffer to front buffer
}



int main(void)
{
	GLFWwindow* window; //Pointer to object that represents the application window

	glfwSetErrorCallback(error_callback);//Register error processing callback procedure

	if (!glfwInit()) { //Initialize GLFW library
		fprintf(stderr, "Can't initialize GLFW.\n");
		exit(EXIT_FAILURE);
	}
	
	window = glfwCreateWindow(720, 720, "OpenGL", NULL, NULL);  //Create a window 500pxx500px titled "OpenGL" and an OpenGL context associated with it.

	if (!window) //If no window is opened then close the program
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Since this moment OpenGL context corresponding to the window is active and all OpenGL calls will refer to this context.
	glfwSwapInterval(1); //During vsync wait for the first refresh

	GLenum err;
	if ((err=glewInit()) != GLEW_OK) { //Initialize GLEW library
		fprintf(stderr, "Can't initialize GLEW: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Call initialization procedure


	float angle_x=0; //current rotation angle of the object, x axis
	float angle_y=0; //current rotation angle of the object, y axis
	glfwSetTime(0); //Zero the timer
	//Main application loop
	while (!glfwWindowShouldClose(window)) //As long as the window shouldnt be closed yet...
	{
		if (!right_button_pressed) {
			speed_x = 0;
			speed_y = 0;
		}

        angle_x+=speed_x*glfwGetTime(); //Add angle by which the object was rotated in the previous iteration
		angle_y+=speed_y*glfwGetTime(); //Add angle by which the object was rotated in the previous iteration
        glfwSetTime(0); //Zero the timer
		drawScene(window,angle_x,angle_y); //Execute drawing procedure
		glfwPollEvents(); //Process callback procedures corresponding to the events that took place up to now
	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Delete OpenGL context and the window.
	glfwTerminate(); //Free GLFW resources
	exit(EXIT_SUCCESS);
}
