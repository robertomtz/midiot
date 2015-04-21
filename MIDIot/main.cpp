#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "RtMidi.h"
bool done;
static void finish(int ignore){ done = true; }

int notaCordenada[15]={190, 202, 215, 227, 240, 252 ,265, 290, 317, 329, 342, 354, 367, 379};
std::string notaNombre[15]={"C3","D3","E3","F3","G3","A4","B4","C4","D4","E4","F4","G4","A5","B5"};

RtMidiIn  *midiin = 0;
std::vector<unsigned char> message;
int nBytes, i;
double stamp;

int score=0;
double tiempo=60;
int an=640,al=480;
int posXNotes=-400;
bool start=false;

void myTimer(int v)
{
    if(start){
        tiempo-=.01;
    }
    if (tiempo==0){
        start=false;
    }
    
    if ( !done ) {
        stamp = midiin->getMessage( &message );
        nBytes = message.size();
        for ( i=0; i<nBytes; i++ )
            std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
        if ( nBytes > 0 )
            std::cout << "stamp = " << stamp << std::endl;
        // Sleep for 10 milliseconds ... platform-dependent.
    }
    
    glutPostRedisplay();
    glutTimerFunc(10, myTimer, 1);
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

std::string toString(double value) {
    std::stringstream ss;
    ss << value;
    return ss.str().substr(0,ss.str().find('.'));
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
    
    //Las demas notas
    for (int i=0; i<15; i++) {
        glPushMatrix();
        glTranslatef(posXNotes+40*i, notaCordenada[i], 0);
        glutSolidSphere(12,12, 12);
        glPopMatrix();
    }
 
    drawText(4000, 4800, .1, "MIDI OT", GLUT_BITMAP_9_BY_15);
    
    if(!start){
        drawText(-300, 0, 1, "PRESS S", GLUT_BITMAP_9_BY_15);
        drawText(-300, -150, 1, "TO START", GLUT_BITMAP_9_BY_15);
    }
    
    drawText(-500, -480, 1, toString(tiempo), GLUT_BITMAP_9_BY_15);//time
    drawText(100, -480, 1, toString(score), GLUT_BITMAP_9_BY_15);//score
    
//    rectangulo fondo blanco
    glColor3f(1, 1, 1);
    glRectd(-an, -an, an, an);
    
    
    glutSwapBuffers();
}

void myKey(unsigned char theKey, int mouseX, int mouseY)
{
    switch (theKey)
    {
        case 's':
        case 'S':
            if(!start){
                start=true;
                tiempo=60;
            }
            break;
        case 'p':
        case 'P':
            start = false;
        case 27:
            exit(-1);
            //terminate the program
            break;
            
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
    // RtMidiIn constructor
    try {
        midiin = new RtMidiIn();
    }
    catch ( RtMidiError &error ) {
        error.printMessage();
        exit( EXIT_FAILURE );
    }
    // Check inputs.
    unsigned int nPorts = midiin->getPortCount();
    std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
    std::string portName;
    for ( unsigned int i=0; i<nPorts; i++ ) {
        try {
            portName = midiin->getPortName(i);
        }
        catch ( RtMidiError &error ) {
            error.printMessage();
        }
        std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
    }

    
    midiin->openPort( 0 );
    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes( false, false, false );
    // Install an interrupt handler function.
    done = false;
    (void) signal(SIGINT, finish);
    // Periodically check input queue.
    std::cout << "Reading MIDI from port ...";
    
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

