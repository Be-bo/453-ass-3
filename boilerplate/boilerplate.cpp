// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Co-Authors:
//			Jeremy Hart, University of Calgary
//			John Hall, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <math.h>

#include "texture.h"

#include "GlyphExtractor.h"

using namespace std;
using namespace glm;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint tcsShader, GLuint tesShader);

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

// load, compile, and link shaders, returning true if successful
GLuint InitializeShaders2()
{
	// load shader source from files
	string vertexSource = LoadSource("shaders/vertex2.glsl");
	string fragmentSource = LoadSource("shaders/fragment.glsl");
	if (vertexSource.empty() || fragmentSource.empty()) return false;

	// compile shader source into shader objects
	GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	// link shader program
	GLuint program = LinkProgram(vertex, fragment, 0, 0);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// check for OpenGL errors and return false if error occurred
	return program;
}

// load, compile, and link shaders, returning true if successful
GLuint InitializeShaders()
{
	// load shader source from files
	string vertexSource = LoadSource("shaders/vertex.glsl");
	string fragmentSource = LoadSource("shaders/fragment.glsl");
        string tcsSource = LoadSource("shaders/tessControl.glsl");
        string tesSource = LoadSource("shaders/tessEval.glsl");
        
	if (vertexSource.empty() || fragmentSource.empty()) return 0;

	// compile shader source into shader objects
	GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	GLuint tcs = CompileShader(GL_TESS_CONTROL_SHADER, tcsSource);
	GLuint tes = CompileShader(GL_TESS_EVALUATION_SHADER, tesSource);
	
	GLuint program = LinkProgram(vertex, fragment, tcs, tes);

	glDeleteShader(vertex);
	glDeleteShader(fragment);	
        glDeleteShader(tcs);
	glDeleteShader(tes);

	if (CheckGLErrors())
		return 0;

	// check for OpenGL errors and return false if error occurred
        return program;
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct Geometry
{
	// OpenGL names for array buffer objects, vertex array object
	GLuint  vertexBuffer;
	GLuint  textureBuffer;
	GLuint  colourBuffer;
	GLuint  vertexArray;
	GLsizei elementCount;

	// initialize object names to zero (OpenGL reserved value)
	Geometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
	{}
};

bool InitializeVAO(Geometry *geometry){

	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;

	//Generate Vertex Buffer Objects
	// create an array buffer object for storing our vertices
	glGenBuffers(1, &geometry->vertexBuffer);

	// create another one for storing our colours
	glGenBuffers(1, &geometry->colourBuffer);

	//Set up Vertex Array Object
	// create a vertex array object encapsulating all our vertex attributes
	glGenVertexArrays(1, &geometry->vertexArray);
	glBindVertexArray(geometry->vertexArray);

	// associate the position array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glVertexAttribPointer(
		VERTEX_INDEX,		//Attribute index 
		2, 					//# of components
		GL_FLOAT, 			//Type of component
		GL_FALSE, 			//Should be normalized?
		sizeof(vec2),		//Stride - can use 0 if tightly packed
		0);					//Offset to first element
	glEnableVertexAttribArray(VERTEX_INDEX);

	// associate the colour array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glVertexAttribPointer(
		COLOUR_INDEX,		//Attribute index 
		3, 					//# of components
		GL_FLOAT, 			//Type of component
		GL_FALSE, 			//Should be normalized?
		sizeof(vec3), 		//Stride - can use 0 if tightly packed
		0);					//Offset to first element
	glEnableVertexAttribArray(COLOUR_INDEX);

	// unbind our buffers, resetting to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return !CheckGLErrors();
}

// create buffers and fill with geometry data, returning true if successful
bool LoadGeometry(Geometry *geometry, vec2 *vertices, vec3 *colours, int elementCount)
{
	geometry->elementCount = elementCount;

	// create an array buffer object for storing our vertices
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*geometry->elementCount, vertices, GL_STATIC_DRAW);

	// create another one for storing our colours
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*geometry->elementCount, colours, GL_STATIC_DRAW);

	//Unbind buffer to reset to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(Geometry *geometry)
{
	// unbind and destroy our vertex array object and associated buffers
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &geometry->vertexArray);
	glDeleteBuffers(1, &geometry->vertexBuffer);
	glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(Geometry *geometry, GLuint program, int type)
{

	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glUseProgram(program);
	glBindVertexArray(geometry->vertexArray);

        if(type == 0){
                glDrawArrays(GL_PATCHES, 0, geometry->elementCount);
        }else if(type == 1){
	        glDrawArrays(GL_LINE_STRIP, 0, geometry->elementCount);
        }else if(type == 2){
                glDrawArrays(GL_POINTS, 0, geometry->elementCount);
        }

	// reset state to default (no shader or geometry bound)
	glBindVertexArray(0);
	glUseProgram(0);

	// check for an report any OpenGL errors
	CheckGLErrors();
}

// -------------------------------------------------------------------------- vec2 position = (1-u)*(1-u)*(1-u)*p0 + 3*u*(1-u)*(1-u)*p1 + 3*u*u*(1-u)*p2 + u*u*u*p3; 
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}



//GLOBAL VARS
int sceneId = 0;

//KEY INPUT
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

        if(action == GLFW_PRESS){
		if(key == GLFW_KEY_B){ //mug
                        sceneId = 0;                
                }else if(key == GLFW_KEY_N){ //fish
                        sceneId = 1; 
                }else if(key == GLFW_KEY_F){
                        sceneId = 2; //font Sans Pro
                }else if(key == GLFW_KEY_G){
                        sceneId = 3; //font Lora
                }else if(key == GLFW_KEY_H){
                        sceneId = 4; //font Inconsolata
                }          
	}
}


//EXTRACT FONT
void extractLetter(vector<vec2>*rPointsLocal, vector<vec3>* rColorsLocal, char letter, string fontString){
        GlyphExtractor extractor;
        extractor.LoadFontFile(fontString);
        MyGlyph rGlyph = extractor.ExtractGlyph(letter);
        for(int i = 0; i<rGlyph.contours.size(); i++){
                for(int j = 0; j<rGlyph.contours[i].size(); j++){

                        for(int k = 0; k<rGlyph.contours[i][j].degree; k++){
                        
                        rPointsLocal->push_back(vec2(rGlyph.contours[i][j].x[k], rGlyph.contours[i][j].y[k]));
                        rColorsLocal->push_back(vec3(1.0,0.0,0.0));
                        }
                        for (int k = 0; k<4-rGlyph.contours[i][j].degree; k++){
                             rPointsLocal->push_back(vec2(rGlyph.contours[i][j].x[rGlyph.contours[i][j].degree], rGlyph.contours[i][j].y[rGlyph.contours[i][j].degree]));
                             rColorsLocal->push_back(vec3(1.0,0.0,0.0));                           
                        }
                }
        }

}


void extractFont(vector<vec2>*fontPoints, vector<vec3>* fontColors, string fontString){
        vector<vec2> RPoints;
	vector<vec3> RColors;
        vector<vec2> oPoints;
	vector<vec3> oColors;
        vector<vec2> bPoints;
	vector<vec3> bColors;
        vector<vec2> ePoints;
	vector<vec3> eColors;
        vector<vec2> rPoints;
	vector<vec3> rColors;
        vector<vec2> tPoints;
	vector<vec3> tColors;

        extractLetter(&RPoints, &RColors, 'R', fontString);
        extractLetter(&oPoints, &oColors, 'o', fontString);
        extractLetter(&bPoints, &bColors, 'b', fontString);
        extractLetter(&ePoints, &eColors, 'e', fontString);
        extractLetter(&rPoints, &rColors, 'r', fontString);
        extractLetter(&tPoints, &tColors, 't', fontString);

        for(int i = 0; i<RPoints.size(); i++){
                RPoints.at(i) = RPoints.at(i)/2.f-vec2(0.8,0.1);
                fontPoints->push_back(RPoints.at(i));
                fontColors->push_back(vec3(1.f,0.f,0.f));
        }
        for(int i = 0; i<oPoints.size(); i++){
                oPoints.at(i) = oPoints.at(i)/2.f-vec2(0.5,0.1);
                fontPoints->push_back(oPoints.at(i));
                fontColors->push_back(vec3(1.f,0.f,0.f));
        }
        for(int i = 0; i<bPoints.size(); i++){
                bPoints.at(i) = bPoints.at(i)/2.f-vec2(0.2,0.1);
                fontPoints->push_back(bPoints.at(i));
                fontColors->push_back(vec3(1.f,0.f,0.f));
        }
        for(int i = 0; i<ePoints.size(); i++){
                ePoints.at(i) = ePoints.at(i)/2.f-vec2(-0.1,0.1);
                fontPoints->push_back(ePoints.at(i));
                fontColors->push_back(vec3(1.f,0.f,0.f));
        }
        for(int i = 0; i<rPoints.size(); i++){
                rPoints.at(i) = rPoints.at(i)/2.f-vec2(-0.4,0.1);
                fontPoints->push_back(rPoints.at(i));
                fontColors->push_back(vec3(1.f,0.f,0.f));
        }
        for(int i = 0; i<tPoints.size(); i++){
                tPoints.at(i) = tPoints.at(i)/2.f-vec2(-0.65,0.1);
                fontPoints->push_back(tPoints.at(i));
                fontColors->push_back(vec3(1.f,0.f,0.f));
        }     
}


//COFFEE
void mug(vector<vec2>* vertices, vector<vec3>* colours, vector<vec2>* verticesControl, vector<vec3>* coloursControl, vector<vec2>* verticesControlPoints, vector<vec3>* coloursControlPoints){
       
        //mug colors and  vertices
        vertices->push_back(vec2(1.f/3.f, 1.f/3.f));
        vertices->push_back(vec2(2.f/3.f, -1.f/3.f));
        vertices->push_back(vec2(0,-1.f/3.f));
        vertices->push_back(vec2(0,0));

        vertices->push_back(vec2(0,-1.f/3.f));
        vertices->push_back(vec2(-2.f/3.f,-1.f/3.f));
        vertices->push_back(vec2(-1.f/3.f,1.f/3.f));
        vertices->push_back(vec2(0,0));

        vertices->push_back(vec2(-1.f/3.f,1.f/3.f));
        vertices->push_back(vec2(0,1.f/3.f));
        vertices->push_back(vec2(1/3.f,1.f/3.f));
        vertices->push_back(vec2(0,0));
        
        vertices->push_back(vec2(0.4,0.5/3.f));
        vertices->push_back(vec2(2.5/3.f,1.f/3.f));
        vertices->push_back(vec2(1.3/3.f,-0.4/3.f));
        vertices->push_back(vec2(0,0));

        for(int i = 0; i<16; i++) colours->push_back(vec3(1.0f, 0.0f, 1.0f));
        

        //control points of the control polygon and its colors
        verticesControl->push_back(vec2(1.f/3.f, 1.f/3.f));
        for(int i = 0; i<2; i++) verticesControl->push_back(vec2(2.f/3.f, -1.f/3.f));
        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(0,-1.f/3.f));

        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(0,-1.f/3.f));
        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(-2.f/3.f,-1.f/3.f));
        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(-1.f/3.f,1.f/3.f));

        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(-1.f/3.f,1.f/3.f));
        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(0,1.f/3.f));
        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(1.f/3.f,1.f/3.f));
        
        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(0.4,0.5/3.f));
        for(int i = 0; i<2; i++)verticesControl->push_back(vec2(2.5/3.f,1.f/3.f));
        verticesControl->push_back(vec2(1.3/3.f,-0.4/3.f));

        for(int i = 0; i<22; i++) coloursControl->push_back(vec3(0.0f, 0.0f, 1.0f));
        

        //control points and colours
        verticesControlPoints->push_back(vec2(1.f/3.f, 1.f/3.f));
        verticesControlPoints->push_back(vec2(2.f/3.f, -1.f/3.f));
        verticesControlPoints->push_back(vec2(0,-1.f/3.f));

        verticesControlPoints->push_back(vec2(0,-1.f/3.f));
        verticesControlPoints->push_back(vec2(-2.f/3.f,-1.f/3.f));
        verticesControlPoints->push_back(vec2(-1.f/3.f,1.f/3.f));

        verticesControlPoints->push_back(vec2(-1.f/3.f,1.f/3.f));
        verticesControlPoints->push_back(vec2(0,1.f/3.f));
        verticesControlPoints->push_back(vec2(1.f/3.f,1.f/3.f));
        
        verticesControlPoints->push_back(vec2(0.4,0.5/3.f));
        verticesControlPoints->push_back(vec2(2.5/3.f,1.f/3.f));
        verticesControlPoints->push_back(vec2(1.3/3.f,-0.4/3.f));
        
        for(int i = 0; i<4; i++){
                coloursControlPoints->push_back(vec3(1.0f, 0.0f, 0.0f));
                coloursControlPoints->push_back(vec3(1.0f, 1.0f, 1.0f));
                coloursControlPoints->push_back(vec3(1.0f, 0.0f, 0.0f));
        }
        
}

//FISH
void fish(vector<vec2>* vertices, vector<vec3>* colours, vector<vec2>* verticesControl, vector<vec3>* coloursControl, vector<vec2>* verticesControlPoints, vector<vec3>* coloursControlPoints){
        
        //shift causes some trouble we need to account for that by clearing the arrays
        vertices->clear();
        colours->clear();
        verticesControl->clear();
        coloursControl->clear();
        verticesControlPoints->clear();
        coloursControlPoints->clear();        

        //fish colors and  vertices
        vertices->push_back(vec2(1.f/6.f, 1.f/6.f));
        vertices->push_back(vec2(4.f/6.f, 0));
        vertices->push_back(vec2(6.f/6.f, 2.f/6.f));
        vertices->push_back(vec2(9.f/6.f, 1.f/6.f));

        vertices->push_back(vec2(8.f/6.f, 2.f/6.f));
        vertices->push_back(vec2(0, 8.f/6.f));
        vertices->push_back(vec2(0, -2.f/6.f));
        vertices->push_back(vec2(8.f/6.f, 4.f/6.f));

        vertices->push_back(vec2(5.f/6.f, 3.f/6.f));
        vertices->push_back(vec2(3.f/6.f, 2.f/6.f));
        vertices->push_back(vec2(3.f/6.f, 3.f/6.f));
        vertices->push_back(vec2(5.f/6.f, 2.f/6.f));

        vertices->push_back(vec2(3.f/6.f, 2.2/6.f));
        vertices->push_back(vec2(3.5/6.f, 2.7/6.f));
        vertices->push_back(vec2(3.5/6.f, 3.3/6.f));
        vertices->push_back(vec2(3.f/6.f, 3.8/6.f));

        vertices->push_back(vec2(2.8/6.f, 3.5/6.f));
        vertices->push_back(vec2(2.4/6.f, 3.8/6.f));
        vertices->push_back(vec2(2.4/6.f, 3.2/6.f));
        vertices->push_back(vec2(2.8/6.f, 3.5/6.f));
        
        for(int i = 0; i<vertices->size();i++){ //our shift to make it nicer
                vertices->at(i) = vertices->at(i) - vec2(0.75, 0.5);
        }

        for(int i = 0; i<20; i++) colours->push_back(vec3(1.0f, 0.0f, 1.0f));
        

        //control points of the control polygon and its colors
        verticesControl->push_back(vec2(1.f/6.f, 1.f/6.f));
        verticesControl->push_back(vec2(4.f/6.f, 0));
        verticesControl->push_back(vec2(6.f/6.f, 2.f/6.f));
        verticesControl->push_back(vec2(9.f/6.f, 1.f/6.f));

        verticesControl->push_back(vec2(8.f/6.f, 2.f/6.f));
        verticesControl->push_back(vec2(0, 8.f/6.f));
        verticesControl->push_back(vec2(0, -2.f/6.f));
        verticesControl->push_back(vec2(8.f/6.f, 4.f/6.f));

        verticesControl->push_back(vec2(5.f/6.f, 3.f/6.f));
        verticesControl->push_back(vec2(3.f/6.f, 2.f/6.f));
        verticesControl->push_back(vec2(3.f/6.f, 3.f/6.f));
        verticesControl->push_back(vec2(5.f/6.f, 2.f/6.f));

        verticesControl->push_back(vec2(3.f/6.f, 2.2/6.f));
        verticesControl->push_back(vec2(3.5/6.f, 2.7/6.f));
        verticesControl->push_back(vec2(3.5/6.f, 3.3/6.f));
        verticesControl->push_back(vec2(3.f/6.f, 3.8/6.f));

        verticesControl->push_back(vec2(2.8/6.f, 3.5/6.f));
        verticesControl->push_back(vec2(2.4/6.f, 3.8/6.f));
        verticesControl->push_back(vec2(2.4/6.f, 3.2/6.f));
        verticesControl->push_back(vec2(2.8/6.f, 3.5/6.f));
        
        for(int i = 0; i<verticesControl->size();i++){
                verticesControl->at(i) = verticesControl->at(i) - vec2(0.75, 0.5);
        }

        for(int i = 0; i<20; i++) coloursControl->push_back(vec3(0.0f, 0.0f, 1.0f));
        

        //control points and colours
        verticesControlPoints->push_back(vec2(1.f/6.f, 1.f/6.f));
        verticesControlPoints->push_back(vec2(4.f/6.f, 0));
        verticesControlPoints->push_back(vec2(6.f/6.f, 2.f/6.f));
        verticesControlPoints->push_back(vec2(9.f/6.f, 1.f/6.f));

        verticesControlPoints->push_back(vec2(8.f/6.f, 2.f/6.f));
        verticesControlPoints->push_back(vec2(0, 8.f/6.f));
        verticesControlPoints->push_back(vec2(0, -2.f/6.f));
        verticesControlPoints->push_back(vec2(8.f/6.f, 4.f/6.f));

        verticesControlPoints->push_back(vec2(5.f/6.f, 3.f/6.f));
        verticesControlPoints->push_back(vec2(3.f/6.f, 2.f/6.f));
        verticesControlPoints->push_back(vec2(3.f/6.f, 3.f/6.f));
        verticesControlPoints->push_back(vec2(5.f/6.f, 2.f/6.f));

        verticesControlPoints->push_back(vec2(3.f/6.f, 2.2/6.f));
        verticesControlPoints->push_back(vec2(3.5/6.f, 2.7/6.f));
        verticesControlPoints->push_back(vec2(3.5/6.f, 3.3/6.f));
        verticesControlPoints->push_back(vec2(3.f/6.f, 3.8/6.f));

        verticesControlPoints->push_back(vec2(2.8/6.f, 3.5/6.f));
        verticesControlPoints->push_back(vec2(2.4/6.f, 3.8/6.f));
        verticesControlPoints->push_back(vec2(2.4/6.f, 3.2/6.f));
        verticesControlPoints->push_back(vec2(2.8/6.f, 3.5/6.f));
        
        for(int i = 0; i<verticesControlPoints->size();i++){
                verticesControlPoints->at(i) = verticesControlPoints->at(i) - vec2(0.75, 0.5);
        }

        for(int i = 0; i<5; i++){
                coloursControlPoints->push_back(vec3(1.0f, 0.0f, 0.0f));
                coloursControlPoints->push_back(vec3(1.0f, 1.0f, 1.0f));
                coloursControlPoints->push_back(vec3(1.0f, 1.0f, 1.0f));
                coloursControlPoints->push_back(vec3(1.0f, 0.0f, 0.0f));
        }
        
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
	// initialize the _FW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);
	int width = 512, height = 512;
	window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, KeyCallback);
	glfwMakeContextCurrent(window);

	//Intialize GLAD
	if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}

	// query and print out information about our OpenGL environment
	QueryGLVersion();

	// call function to load and compile SHADER PROGRAMS!!!
	GLuint program = InitializeShaders();
	if (program == 0) {
		cout << "Program could not initialize shaders, TERMINATING" << endl;
		return -1;
	}

        GLuint program2 = InitializeShaders2();
	if (program2 == 0) {
		cout << "Program could not initialize shaders, TERMINATING" << endl;
		return -1;
	}

        GLuint program3 = InitializeShaders2();
	if (program3 == 0) {
		cout << "Program could not initialize shaders, TERMINATING" << endl;
		return -1;
	}



        //INITIAL VALUES FOR EVERYTHING
        vector<vec2> vertices;
	vector<vec3> colours;
        vector<vec2> verticesControl;
	vector<vec3> coloursControl;
        vector<vec2> verticesControlPoints;
	vector<vec3> coloursControlPoints;
        vector<vec2> fontPoints;
	vector<vec3> fontColors;

        int patchSize = 4;
        int lastScene = -1;

        
             

	// call function to create and fill buffers with geometry data
	Geometry geometry;
        Geometry geometryControl; //everything that has "Control" at the end refers to the control polygon
        Geometry geometryControlPoints;
        Geometry geometryGlyph;


        if (!InitializeVAO(&geometryGlyph))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&geometryGlyph, fontPoints.data(), fontColors.data(), fontPoints.size()))
		cout << "Failed to load geometry" << endl;
	
	glPatchParameteri(GL_PATCH_VERTICES, patchSize);

	if (!InitializeVAO(&geometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&geometry, vertices.data(), colours.data(), vertices.size()))
		cout << "Failed to load geometry" << endl;
	
	glPatchParameteri(GL_PATCH_VERTICES, patchSize);

        if (!InitializeVAO(&geometryControl))
		cout << "Program failed to intialize geometry 2!" << endl;

        if(!LoadGeometry(&geometryControl, verticesControl.data(), coloursControl.data(), verticesControl.size()))
		cout << "Failed to load geometry" << endl;

        if (!InitializeVAO(&geometryControlPoints))
		cout << "Program failed to intialize geometry 3!" << endl;

        if(!LoadGeometry(&geometryControlPoints, verticesControlPoints.data(), coloursControlPoints.data(), verticesControlPoints.size()))
		cout << "Failed to load geometry" << endl;



        // run an event-triggered main loop
        glPointSize(5);
	while (!glfwWindowShouldClose(window))
	{
                glUseProgram(program);                

                if(lastScene != sceneId){
                       cout<<"changing"<<endl;
                       glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	               glClear(GL_COLOR_BUFFER_BIT);
                       vertices.clear();
                       colours.clear();
                       verticesControl.clear();
                       coloursControl.clear();
                       verticesControlPoints.clear();
                       coloursControlPoints.clear();
                       fontPoints.clear();
                       fontColors.clear();
                       
                       lastScene = sceneId;        
                }

                 //SCENE SELECTION
                if(sceneId == 0){ //mug
                        mug(&vertices, &colours, &verticesControl, &coloursControl, &verticesControlPoints, &coloursControlPoints);
                        patchSize = 4;
                        LoadGeometry(&geometry, vertices.data(), colours.data(), vertices.size());
                        glPatchParameteri(GL_PATCH_VERTICES, patchSize);
                        LoadGeometry(&geometryControl, verticesControl.data(), coloursControl.data(), verticesControl.size());
                        LoadGeometry(&geometryControlPoints, verticesControlPoints.data(), coloursControlPoints.data(), verticesControlPoints.size());
                        RenderScene(&geometry, program, 0);
                        RenderScene(&geometryControl, program2, 1);
                        RenderScene(&geometryControlPoints, program3, 2);
                       
     
                }else if(sceneId == 1){ //fish
                        fish(&vertices, &colours, &verticesControl, &coloursControl, &verticesControlPoints, &coloursControlPoints);
                        patchSize = 4;
                        LoadGeometry(&geometry, vertices.data(), colours.data(), vertices.size());
                        glPatchParameteri(GL_PATCH_VERTICES, patchSize);
                        LoadGeometry(&geometryControl, verticesControl.data(), coloursControl.data(), verticesControl.size());
                        LoadGeometry(&geometryControlPoints, verticesControlPoints.data(), coloursControlPoints.data(), verticesControlPoints.size());
                        RenderScene(&geometry, program, 0);
                        RenderScene(&geometryControl, program2, 1);
                        RenderScene(&geometryControlPoints, program3, 2);
                        
                }else if(sceneId == 2){ //sans
                        extractFont(&fontPoints, &fontColors, "SourceSansPro-Regular.otf");
                        LoadGeometry(&geometryGlyph, fontPoints.data(), fontColors.data(), fontPoints.size());
                        RenderScene(&geometryGlyph, program, 0);
                }else if(sceneId == 3){ //lora
                        extractFont(&fontPoints, &fontColors, "Lora-Regular.ttf");
                        LoadGeometry(&geometryGlyph, fontPoints.data(), fontColors.data(), fontPoints.size()); 
                        RenderScene(&geometryGlyph, program, 0);               
                }else if(sceneId == 4){ //inconsolata
                        extractFont(&fontPoints, &fontColors, "Inconsolata.otf");
                        LoadGeometry(&geometryGlyph, fontPoints.data(), fontColors.data(), fontPoints.size()); 
                        RenderScene(&geometryGlyph, program, 0);               
                }


		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	// clean up allocated resources before exit
	DestroyGeometry(&geometry);
	glUseProgram(0);
	glDeleteProgram(program);
	glfwDestroyWindow(window);
	glfwTerminate();

	cout << "Goodbye!" << endl;
	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "OpenGL ERROR:  ";
		switch (flag) {
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM" << endl; break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE" << endl; break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION" << endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY" << endl; break;
		default:
			cout << "[unknown error code]" << endl;
		}
		error = true;
	}
	return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
	string source;

	ifstream input(filename.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	}
	else {
		cout << "ERROR: Could not load shader source from file "
			<< filename << endl;
	}

	return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
	// allocate shader object name
	GLuint shaderObject = glCreateShader(shaderType);

	// try compiling the source as a shader of the given type
	const GLchar *source_ptr = source.c_str();
	glShaderSource(shaderObject, 1, &source_ptr, 0);
	glCompileShader(shaderObject);

	// retrieve compile status
	GLint status;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
		cout << "ERROR compiling shader:" << endl << endl;
		cout << source << endl;
		cout << info << endl;
	}

	return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint tcsShader, GLuint tesShader)
{
	// allocate program object name
	GLuint programObject = glCreateProgram();

	// attach provided shader objects to this program
	if (vertexShader)   glAttachShader(programObject, vertexShader);
	if (fragmentShader) glAttachShader(programObject, fragmentShader);
	if (tcsShader) glAttachShader(programObject, tcsShader);
	if (tesShader) glAttachShader(programObject, tesShader);

	// try linking the program with given attachments
	glLinkProgram(programObject);

	// retrieve link status
	GLint status;
	glGetProgramiv(programObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
		cout << "ERROR linking shader program:" << endl;
		cout << info << endl;
	}

	return programObject;
}


