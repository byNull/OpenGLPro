// CTest01.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tchar.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glut32.lib")

void init (void)
{
    glClearColor (1.0,1.0,1.0,0.0);

    glMatrixMode (GL_PROJECTION);
    gluOrtho2D (0.0,200.0,0.0,150.0);
}

void lineSegment (void)
{
    glClear (GL_COLOR_BUFFER_BIT);

    glColor3f (1.0,0.0,0.0);
    glBegin (GL_LINES);
           glVertex2i (180,15);
           glVertex2i (10,145);
    glEnd ();

    glFlush ();
}

int _tmain (int argc, _TCHAR* argv[])
{
    glutInit (&argc,argv);
    glutInitDisplayMode (GLUT_SINGLE|GLUT_RGB);
    glutInitWindowPosition (50,100);
    glutInitWindowSize (400,300);
    glutCreateWindow ("阿灰的第一个OpenGL程序");

    init();
    glutDisplayFunc (lineSegment);
    glutMainLoop ();

}


