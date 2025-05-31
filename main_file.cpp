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
#include <Models/OfficeChair/officeChair.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


double speed_x=0; //angular speed in radians
double speed_y=0; //angular speed in radians
double aspectRatio=1;
double camera_distance = 3.0f;
bool right_button_pressed = false;
double last_x = 0, last_y = 0;
ShaderProgram *sp; //Pointer to the shader program
GLuint tex0;
GLuint tex1;
//ŻEBY TO DOBRZE BYŁO ZROBIONE TO TRZEBA ZROBIĆ KLASĘ
std::vector<glm::vec4> verts;
std::vector<glm::vec4> norms;
std::vector<glm::vec2> texCoords;
std::vector<unsigned int> indices;

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

void loadModel(std::string filepath) {
	using namespace tinyobj;
	using namespace std;

	attrib_t attrib;
	vector<shape_t> shapes;
	vector<material_t> materials;
	string err;

	size_t lastSlash = filepath.find_last_of("/\\");
	string basedir = (lastSlash != string::npos) ? filepath.substr(0, lastSlash + 1) : "";

	bool ret = LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str(), basedir.c_str(), NULL, true, true);

	if (!err.empty()) {
		cerr << "TinyObjLoader ERR: " << err << endl;
	}
	if (!ret) {
		cerr << "Failed to load OBJ file: " << filepath << endl;
		return;
	}

	for (size_t s = 0; s < shapes.size(); s++) {
		size_t index_offset = 0;

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			for (int v = 0; v < fv; v++) {
				index_t idx = shapes[s].mesh.indices[index_offset + v];

				float vx = attrib.vertices[3 * idx.vertex_index + 0];
				float vy = attrib.vertices[3 * idx.vertex_index + 1];
				float vz = attrib.vertices[3 * idx.vertex_index + 2];
				verts.push_back(glm::vec4(vx, vy, vz, 1.0f));

				if (!attrib.normals.empty() && idx.normal_index >= 0) {
					float nx = attrib.normals[3 * idx.normal_index + 0];
					float ny = attrib.normals[3 * idx.normal_index + 1];
					float nz = attrib.normals[3 * idx.normal_index + 2];
					norms.push_back(glm::vec4(nx, ny, nz, 0.0f));
				}
				else {
					norms.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
				}

				if (!attrib.texcoords.empty() && idx.texcoord_index >= 0) {
					float tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					float ty = attrib.texcoords[2 * idx.texcoord_index + 1];
					texCoords.push_back(glm::vec2(tx, ty));
				}
				else {
					texCoords.push_back(glm::vec2(0.0f, 0.0f));
				}

				indices.push_back(indices.size());
			}
			index_offset += fv;
		}
	}

	// Wczytaj teksturę jeśli materiał ją zawiera
	if (!materials.empty()) {
		string texname = materials[0].diffuse_texname;
		if (!texname.empty()) {
			string texpath = basedir + texname;
			tex0 = readTexture(texpath.c_str());
			cout << "Załadowano teksturę: " << texpath << endl;
		}
		else {
			cout << "Brak tekstury w materiale!" << endl;
		}
	}
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
	//tex0 = readTexture("metal.png");
	//tex1 = readTexture("sky.png");

	loadModel(std::string("Models/DesignChair/DesignChair1.obj")); 
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
    glVertexAttribPointer(sp->a("vertex"),4,GL_FLOAT,false,0, verts.data()); //Specify source of the data for the attribute vertex


	glEnableVertexAttribArray(sp->a("normal")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, norms.data()); //Specify source of the data for the attribute normal

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, tex0);
	glUniform1i(sp->u("textureMap0"), 10);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glUniform1i(sp->u("textureMap1"), 6);

	glEnableVertexAttribArray(sp->a("texCoord0")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords.data()); //Specify source of the data for the attribute normal

    //glDrawArrays(GL_TRIANGLES,0, vertexCount); //Draw the object
	glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,indices.data());
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
