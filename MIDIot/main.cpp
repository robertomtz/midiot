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
#include <time.h>
#include "RtMidi.h"
#include "imageLoader.h"

bool done;
static void finish(int ignore){ done = true; }
static GLuint texName[5];


int notaCordenada[25]={190, 190, 202, 202, 215, 227, 227, 240, 240, 252, 252 , 265, 290, 290, 317, 317, 329, 342, 342, 354, 354, 367, 367, 379, 392};
std::string notaNombre[25]={"C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A4", "A#4", "B4", "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A5", "A#5", "B5", "C5"};

RtMidiIn  *midiin = 0;
std::vector<unsigned char> message;
int nBytes;
double stamp;
bool midiConnected = false;
std::string portName;

bool oprimidoMidi=false;
bool notePressed=false;
bool pausa=false;
bool entrar=false;

int score=0;
double tiempo=60;
int an=640,al=480;
int posXNotes=-350;
bool start=false;

int notaActual=0;
int notaOprimidaActual=-1;
bool entraUno= false;



void loadTexture(Image* image,int k)
{
    glBindTexture(GL_TEXTURE_2D, texName[k]); //Tell OpenGL which texture to edit

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 image->width, image->height,  //Width and height
                 0,                            //The border of the image
                 GL_RGB, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                 //as unsigned numbers
                 image->pixels);               //The actual pixel data
}

void initRendering()
{
    GLuint i=0;
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); ///Users/mariaroque/Imagenes
    // glEnable(GL_COLOR_MATERIAL);

    glGenTextures(5, texName); //Make room for our texture
    Image* image;
    image = loadBMP("/Users/Adrian/Copy/ITC/8to-Semestre/Graficas/midiot/MIDIot/sol.bmp");
    loadTexture(image, 0);

    image = loadBMP("/Users/Adrian/Copy/ITC/8to-Semestre/Graficas/midiot/MIDIot/fa.bmp");
    loadTexture(image, 1);
    delete image;
}

int getRanNumber() {
    return rand() % 24;
}

void createRtMidiIn(){
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
    if (nPorts > 0) {
        midiConnected = true;
    }

    for ( unsigned int i=0; i<nPorts; i++ ) {
        try {
            portName = midiin->getPortName(i);
        }
        catch ( RtMidiError &error ) {
            error.printMessage();
        }
        std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
    }

    if (midiConnected) {
        midiin->openPort( 0 );
        // Don't ignore sysex, timing, or active sensing messages.
        midiin->ignoreTypes( false, false, false );
        // Install an interrupt handler function.
        done = false;
        (void) signal(SIGINT, finish);
        // Periodically check input queue.
        std::cout << "Reading MIDI from port ...";
    } else {
        portName = "No MIDI device connected";
    }
}

void myTimer(int v)
{
    if(start && !pausa){
        tiempo-=0.01;
    }
    if (tiempo<=0.01){
        start=false;
    }

    if ( (!done) && midiConnected ) {
        stamp = midiin->getMessage( &message );
        nBytes = message.size();
        for ( int i=0; i<nBytes; i++ )
            std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
        if ( nBytes > 0 ){
            notaOprimidaActual=(int)message[1]-48;
            std::cout << "stamp = " << stamp << std::endl;
            if(start && !pausa){
                oprimidoMidi=true;
                notePressed=!notePressed;
                if(notePressed){
                    entraUno=false;
                    posXNotes+=50;
                    if(notaOprimidaActual==notaActual){
                        score+=5000;
                    } else{
                        score-=4000;
                    }
                    if (posXNotes==500) {
                        posXNotes=-350;
                    }
                    notaActual=getRanNumber();
                }
            }
        }
    }else {
        createRtMidiIn();
    }
    
    glutPostRedisplay();
    glutTimerFunc(10, myTimer, 1);
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
void playSound(){
    #ifdef DEBUG
        #ifdef ROBI
            std::string path = "/Users/robertomtz/Desktop/Graficas/MIDIot/MIDIot/piano/";
        #else
            std::string path = "/Users/Adrian/Copy/ITC/8to-Semestre/Graficas/midiot/MIDIot/piano/";
        #endif
    #else
        std::string path = "piano/";
    #endif

    std::string cmd;
    cmd = "afplay -q 1 " + path + notaNombre[notaOprimidaActual] + ".wav & exit";
    system(cmd.c_str());
}
void dibuja()
{
    //puntos actuales jugador y dealer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);


    glLineWidth(2);
    glColor3f(0.0, 0.0, 0.0);

    if (entrar){
        glColor3f(1.0, 1.0, 1.0);
        glBindTexture(GL_TEXTURE_2D, texName[0]);
        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-470.0f, 440, 0);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-470.0f, 320.0f, 0);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-380.0f, 320.0f, 0);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-380.0f, 440.0f, 0);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, texName[1]);
        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-470.0f, 265, 0);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-470.0f, 145.0f, 0);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-380.0f, 145.0f, 0);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-380.0f, 265.0f, 0);
        glEnd();


        glColor3f(0.0, 0.0, 0.0);
//       glRectd(-470, 440, -380, 320);
//       glRectd(-470, 265, -380, 145);

        if(!start){
            glColor3f(1.0, 1.0, 1.0);
            drawText(-1650, 250, .3, "Hi superstar! In this game we will show you", GLUT_BITMAP_9_BY_15);
            drawText(-1650, 0, .3, "how to read music from a music sheet.", GLUT_BITMAP_9_BY_15);
            drawText(-1650, -250, .3, "For this, you will only need your", GLUT_BITMAP_9_BY_15);
            drawText(-1650, -500, .3, "MIDI keyboard!", GLUT_BITMAP_9_BY_15);
            drawText(-1650, -750, .3, "Press S when you are ready to start!", GLUT_BITMAP_9_BY_15);
            drawText(-1650, -1000, .3, "Hit the notes as fast you can.", GLUT_BITMAP_9_BY_15);
            drawText(-1650, -1250, .3, "Good luck & stay fingalikin' little chicken!", GLUT_BITMAP_9_BY_15);

            glColor3f(0.0, 0.0, 0.0);
            glRectd(-an, -an, an, 120);
        }


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
    
        //Dibuja todas las notas
    //    for (int i=0; i<25; i++) {
    //        glPushMatrix();
    //        glTranslatef(posXNotes+40*i, notaCordenada[i], 0);
    //        if(notaNombre[i].find("#")!=-1){
    //            drawText(50, 100, .15, "#", GLUT_BITMAP_9_BY_15);
    //        }
    //        glutSolidSphere(12,12, 12);
    //        glPopMatrix();
    //    }
        if(start){
            glPushMatrix();
            glTranslatef(posXNotes, notaCordenada[notaActual], 0);
            if(notaNombre[notaActual].find("#")!=-1){
                drawText(50, 100, .15, "#", GLUT_BITMAP_9_BY_15);
            }
            glutSolidSphere(12,12, 12);
            glPopMatrix();
        }

        drawText(-2000, 1850, 0.25, portName, GLUT_BITMAP_9_BY_15);
        drawText(1300, 1850, 0.25, "MIDI OT", GLUT_BITMAP_9_BY_15);

        if(oprimidoMidi && (!entraUno)){
            entraUno=true;
            if ((int)message[1]-48>=0) {
                drawText(-100, -250, 1, notaNombre[notaOprimidaActual], GLUT_BITMAP_9_BY_15);
                if(start){
                    drawText(-400, -250, 1, notaNombre[notaActual], GLUT_BITMAP_9_BY_15);
                }
                playSound();
                drawText(-400, -250, 1, notaNombre[notaActual], GLUT_BITMAP_9_BY_15);
            }
        }

        drawText(-500, -480, 1, toString(tiempo), GLUT_BITMAP_9_BY_15); //time
        drawText(100, -480, 1, toString(score), GLUT_BITMAP_9_BY_15); //score
    } else{
        glColor3f(1, 1, 1);
        glLineWidth(6);
        drawText(-250, 50, 1.2, "MIDIOT", GLUT_BITMAP_9_BY_15);
        glLineWidth(4);
        drawText(-740, -100, .6, "You better C sharp", GLUT_BITMAP_9_BY_15);
        drawText(-740, -270, .6, "or you will B flat!", GLUT_BITMAP_9_BY_15);
        
        glColor3f(0, 0, 0);
        glLineWidth(4);
        drawText(-400, -670, .4, "Press ENTER to Start", GLUT_BITMAP_9_BY_15);
        glColor3f(0, 0, 0);
        glRectd(-an, -an/3, an, an/3);
    }

//    rectangulo fondo rojo
    glColor3f(0.53, 0.17, 0.18);
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
                score=0;
                notaActual=getRanNumber();
                pausa=false;
            }
            break;
        case 'p':
        case 'P':
            if (start) {
                pausa=!pausa;
            }
        break;
        case 27:
            exit(-1);
            //terminate the program
            break;
            
        case 13:
            entrar=true;
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
    srand (time(NULL));
    createRtMidiIn();

    glutInit(&argc, argv);
    glutInitWindowSize(640,480);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH| GLUT_DOUBLE );
    glutCreateWindow("MIDIot A01190757 - A01190871");
    initRendering();
    glClearColor(1.0,1.0,1.0,1.0);
//    glEnable(GL_DEPTH_TEST);
    glutKeyboardFunc(myKey);
    glutDisplayFunc(dibuja);
    glutTimerFunc(5, myTimer, 1);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
