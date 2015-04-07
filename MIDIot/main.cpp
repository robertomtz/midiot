#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <ctime>

int score=0;
int an=640,al=480;

void myTimer(int v)
{
    glutPostRedisplay();
    glutTimerFunc(5, myTimer, 1);
    
}

void init()
{
    // Para que las paredes se vean sólidas (no transparentes)
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
}


std::string cToString(char a){
    std::stringstream ss;
    ss << a;
    return ss.str();
}

std::string toString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

void drawText(float x, float y, float size, std::string text, void* font) {
    glMatrixMode(GL_MODELVIEW);
    for (std::string::iterator i = text.begin(); i != text.end(); ++i)
    {
        glPushMatrix();
        glScaled(size, size, 0);
        glTranslatef(x, y, 0);
        char c = *i;
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
        glPopMatrix();
        x+=75;
    }
}

void dibuja()
{
    //puntos actuales jugador y dealer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    glLineWidth(2);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    for (int i=0; i<10; i++) {
        if(i<5){
            glVertex3f(-480, 430-i*25, 0);
            glVertex3f(480, 430-i*25, 0);
        }else{
            glVertex3f(-480, 430-i*25-50, 0);
            glVertex3f(480, 430-i*25-50, 0);
        }
    }
    glEnd();
    
    drawText(-500, -480, 1, "60", GLUT_BITMAP_9_BY_15);//score
    drawText(100, -480, 1, "10000", GLUT_BITMAP_9_BY_15);//score
    
//    rectangulo fondo blanco
    glColor3f(1, 1, 1);
    glRectd(-an, -an, an, an);
    
    
    glutSwapBuffers();
}


void myKey(unsigned char theKey, int mouseX, int mouseY){
    switch (theKey)
    {
        default:
            break;		      // do nothing
    }
    glutPostRedisplay();
}


void reshape(int ancho, int alto)
{
    // Ventana
    glViewport(0, 0, ancho, alto);
    // Sistema de coordenadas
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-500, 500, -500, 500,100, 300.0 ); //izq, der, abajo, arriba, cerca, lejos
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 200, 0, 0, 0, 0, 1, 0);
    
}

int main(int argc, char *argv[])
{
    srand (time(NULL));
    glutInit(&argc, argv);
    glutInitWindowSize(640,480);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH| GLUT_DOUBLE );
    glutCreateWindow("MIDIot A01190757 - A01190871");
    init();
    glEnable(GL_DEPTH_TEST);
    glutKeyboardFunc(myKey);
    glutDisplayFunc(dibuja);
    glutTimerFunc(5, myTimer, 1);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}

