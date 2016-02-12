#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>  
#include <stack> 
#include <vector>  
#include "InitShader.h"
#include "vao.h"
#include "MyFile.h"
using namespace std;

MyFile myFile;
static const std::string vertex_shader("lab1_vs.glsl");
static const std::string fragment_shader("lab1_fs.glsl");
GLuint shader_program = -1;
GLuint vao[2] = { -1, -1 };
GLuint vbo[2] = { -1, -1 };
bool bMouseLeftDown = false;
bool bHull = false;
int nMouseX = 0;
int nMouseY = 0;
int nCurPointNum = -1;
int nBeginMousePosX = -1;
int nBeginMousePosY = -1;
int nCurMousePosX = -1;
int nCurMousePosY = -1;
float fWidth = 0;
float fHeight = 0;
enum MouseState
{
	NONE = 0,
	ADD ,	
	DEL,
	MOV,
	HULL
};
MouseState mouseState;

vector<Point3f> vRawPoints,vHull;
Point3f originPoint;
glm::mat4 P;
void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glm::mat4 M = glm::mat4(1.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	glUseProgram(shader_program);
	int PVM_loc = glGetUniformLocation(shader_program, "PVM");
	if (PVM_loc != -1)
	{
		glm::mat4 PVM = P*V*M;
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	}	
	int color_loc = glGetUniformLocation(shader_program, "ucolor");
	glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	if (color_loc != -1)
		glUniform4fv(color_loc, 1, glm::value_ptr(color));
	
	glPointSize(8.0);
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_POINTS, 0, vRawPoints.size());	
	if (bHull)
	{
		color = glm::vec4(0.35f, 0.35f, 0.35f, 0.5f);
		if (color_loc != -1)
			glUniform4fv(color_loc, 1, glm::value_ptr(color));
		glBindVertexArray(vao[1]);
		glDrawArrays(GL_POLYGON, 0, vHull.size());
	}	
	glBindVertexArray(0);	
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	fWidth = float(w);
	fHeight =float(h);
	P = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f);
}

void idle()
{
	glutPostRedisplay();	
}

void reload_shader()
{
   GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());
   if(new_shader == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
   }
   else
   {
      glClearColor(1.0f, 1.0f,1.0f, 1.0f);
      if(shader_program != -1)
      {
         glDeleteProgram(shader_program);
      }
      shader_program = new_shader;    	  
   }
}

void special(int key, int x, int y)
{
	std::cout << "key : " << key << ", x: " << x << ", y: " << y << std::endl;
}

void keyboard(unsigned char key, int x, int y)
{
   std::cout << "key : " << key << ", x: " << x << ", y: " << y << std::endl;
   switch(key)
   {
      case 'r':
      case 'R':
         reload_shader();     
      break;      	  
	  case VK_ESCAPE:
		  ::PostQuitMessage(0);
		  break;
   }
}

void printGlInfo()
{
   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;
}
bool MySort(const Point3f &v1, const Point3f &v2)
{
	return v1.x < v2.x;
}
Point3f ScreenToCoord(Point3f p)
{
	return { (p.x - fWidth / 2.0f) / (fWidth / 2.0f), (fHeight / 2.0f - p.y) / (fHeight / 2.0f) };
}
Point3f CoordToScreen(Point3f p)
{
	return { (1 + p.x)*fWidth / 2.0f, (1 - p.y) * fHeight / 2.0f };
}

//point P is on the left side of p1p2
bool ToLeftTest(Point3f p1, Point3f p2, Point3f p)
{
	Point3f pp1{ 0, 0, 0 }, pp2{ 0, 0, 0 };
	pp1.x = p1.x - p.x;
	pp1.y = p1.y - p.y;
	pp2.x = p2.x - p.x;
	pp2.y = p2.y - p.y;

	return ((pp2.x * pp1.y - pp2.y * pp1.x) > 0);
}
void MakeHull()
{
	int nSize = vRawPoints.size();
	if (nSize < 3) return;

	vector<Point3f> upperHull, lowerHull;
	vector<Point3f> temp = vRawPoints;
	std::sort(temp.begin(), temp.end(), MySort);

	//upper
	upperHull = { temp[0], temp[1] };
	for (int i = 2; i < nSize; i++)
	{
		Point3f p1 = *(upperHull.end() - 2);
		Point3f p2 = *(upperHull.end() - 1);
		Point3f p = temp[i];
		while (ToLeftTest(p1, p2, p))
		{
			upperHull.pop_back();
			if (upperHull.size() == 1) break;
			p1 = *(upperHull.end() - 2);
			p2 = *(upperHull.end() - 1);
		}
		upperHull.push_back(p);
	}
	
	//lower
	lowerHull = { temp[nSize - 1], temp[nSize - 2] };
	for (int i = nSize - 3; i >= 0; i--)
	{
		Point3f p1 = *(lowerHull.end() - 2);
		Point3f p2 = *(lowerHull.end() - 1);
		Point3f p = temp[i];
		while (ToLeftTest(p1, p2, p))
		{
			lowerHull.pop_back();
			if (lowerHull.size() == 1) break;
			p1 = *(lowerHull.end() - 2);
			p2 = *(lowerHull.end() - 1);
		}
		lowerHull.push_back(p);
	}
	vHull = upperHull;
	vHull.insert(vHull.end(), lowerHull.begin(), lowerHull.end());	
}
void RefreshVBO(float dx = 0, float dy = 0)
{
	switch (mouseState)
	{
	case ADD:
	{
				if (vRawPoints.size() == 0) break;
				glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
				glBufferData(GL_ARRAY_BUFFER, vRawPoints.size()*sizeof(Point3f), &vRawPoints[0], GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				if (vHull.size() == 0) break;
				glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
				glBufferData(GL_ARRAY_BUFFER, vHull.size()*sizeof(Point3f), &vHull[0], GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				break;
	}
	case DEL:
	{
				//refesh point
				glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
				GLvoid* data = NULL;
				if (vRawPoints.size() != 0)
					data = &vRawPoints[0];
				else
					data = NULL;
				glBufferData(GL_ARRAY_BUFFER, vRawPoints.size()*sizeof(Point3f), data, GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				//refesh hull
				if (vHull.size() == 0) break;
				MakeHull();
				glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
				glBufferData(GL_ARRAY_BUFFER, vHull.size()*sizeof(Point3f), &vHull[0], GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				break;				
	}
	case MOV:
	{
				//refesh point
				glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
				float *buf = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
				if (buf == NULL) break;								
				buf[nCurPointNum * 3] = originPoint.x + dx;
				buf[nCurPointNum * 3 + 1] = originPoint.y + dy;					
				glUnmapBuffer(GL_ARRAY_BUFFER);

				//refesh hull
				if (vHull.size() == 0) break;
				MakeHull();
				glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
				glBufferData(GL_ARRAY_BUFFER, vHull.size()*sizeof(Point3f), &vHull[0], GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				break;
	}		
	}
	if (vHull.size() < 3)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
		
}
	
int PointerInPoint(int x, int y)
{
	for (int i = 0; i < vRawPoints.size();i++)
	{		
		Point3f p = CoordToScreen(vRawPoints[i]);
		if ((x > p.x - 4 && x < p.x + 4) && (y > p.y - 4 && y < p.y + 4))
		{
			return i;
		}
	}
	return -1;
}
void file_menu(int id)
{
	switch (id)
	{
	case 1:
		vRawPoints = myFile.Open();	
		MakeHull();
		bHull = true;
		mouseState = ADD;
		RefreshVBO();		
		mouseState = NONE;		
		break;
	case 2:
		myFile.Save(vRawPoints);
		break;
	}
}
void points_menu(int id)
{
	switch (id)
	{
	case 1: 
		mouseState = ADD;
		break;
	case 2: 
		mouseState = DEL;
		break;
	case 3: 
		mouseState = MOV;
		break;
	}
}


void main_menu(int id)
{
	MouseState old = mouseState;
	switch (id)
	{
	case 1:		
		mouseState = ADD;		
		if (bHull == false)
			bHull = true;
		MakeHull();			
		break;
	case 2:
		vRawPoints.clear();	
		vHull.clear();
		mouseState = DEL;	
		bHull = false;
		break;
	}
	RefreshVBO();
	mouseState = old;
}
void MouseClick(int button, int state, int x, int y)
{
	nMouseX = x;
	nMouseY = y;

	if (button != GLUT_LEFT_BUTTON) return;	
	if (state == GLUT_DOWN) 
		bMouseLeftDown = true;
	else if (state == GLUT_UP)	
		bMouseLeftDown = false;	

	switch (mouseState)
	{
	case ADD:
	{	
				if (!bMouseLeftDown) break;		
				Point3f p{ (float)x, (float)y };
				vRawPoints.push_back(ScreenToCoord(p));
				RefreshVBO();
	}
	break;	
	case DEL:
	{
				if (!bMouseLeftDown) break;
				nCurPointNum = PointerInPoint(x, y);
				if (nCurPointNum == -1)
					break;				

				vector <Point3f>::iterator Iter;
				int n = 0;
				for (Iter = vRawPoints.begin(); Iter != vRawPoints.end(); Iter++)
				{					
					if (n++ == nCurPointNum)
					{
						vRawPoints.erase(Iter);
						break;
					}
				}
				RefreshVBO();
				break; 
	}
	case MOV:
	{
				if (bMouseLeftDown)//press down
				{				
					nCurPointNum = PointerInPoint(x, y);
					if (nCurPointNum == -1)
						break;
					originPoint = vRawPoints[nCurPointNum];
					nBeginMousePosX = x;
					nBeginMousePosY = y;					
				}
				else//lift up
				{
					if (nCurPointNum == -1)
						break;
					originPoint.x += (nCurMousePosX - nBeginMousePosX)/(fWidth / 2.0f);
					originPoint.y += (nCurMousePosY - nBeginMousePosY) / (-fHeight / 2.0f);
					vRawPoints[nCurPointNum] = originPoint;
				}						
				break;
	}}
}

void MouseMove(int x, int y)
{
	if (!bMouseLeftDown) return;
	if (mouseState != MOV) return;
	nCurMousePosX = x;
	nCurMousePosY = y;

	Point3f newPoint{ 0, 0, 0 };
	float dx = float(x - nBeginMousePosX);
	float dy = float(y - nBeginMousePosY);
	dx /= fWidth / 2.0f;
	dy /= -fHeight / 2.0f;
	newPoint.x = originPoint.x + dx;
	newPoint.y = originPoint.y + dy;
	if (nCurPointNum == -1)
		return;
	vRawPoints[nCurPointNum] = newPoint;

	RefreshVBO(dx , dy );
}
void initOpenGl()
{
   glewInit();
   
   int nFIle_menu = glutCreateMenu(file_menu);
   glutAddMenuEntry("Read", 1);
   glutAddMenuEntry("Write", 2);

   //add menu
   int nback_menu = glutCreateMenu(points_menu);
   glutAddMenuEntry("Add", 1);
   glutAddMenuEntry("Del", 2);
   glutAddMenuEntry("Mov", 3);
   glutAddSubMenu("File", nFIle_menu);
      
   glutCreateMenu(main_menu);
   glutAddSubMenu("Points", nback_menu);
   glutAddMenuEntry("Hull", 1);
   glutAddMenuEntry("Clear", 2);
   glutAttachMenu(GLUT_RIGHT_BUTTON);

   reload_shader();   
   init_buffer();
   mouseState = NONE;

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main (int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv); 
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowPosition (5, 5);
   glutInitWindowSize (600, 600);
   int win = glutCreateWindow ("Jianping Tang. Convex Hull");

   printGlInfo();

   //Register callback functions with glut. 
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);
   glutMouseFunc(MouseClick);
   glutMotionFunc(MouseMove);
   initOpenGl();

   //Enter the glut event loop.
   glutMainLoop();
   glutDestroyWindow(win);
   return 0;		
}

