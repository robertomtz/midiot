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
int posXNotes=-450;
bool start=false;

int notaActual=0;
int notaOprimidaActual=-1;
bool entraUno= false;



void loadTexture(Image* image,int k)
{
    
    glBindTexture(GL_TEXTURE_2D, texName[k]); //Tell OpenGL which texture to edit
    
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
    int i=0;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); ///Users/mariaroque/Imagenes
    // glEnable(GL_COLOR_MATERIAL);
    glGenTextures(5, texName); //Make room for our texture
    Image* image = loadBMP("sol.bmp");
    loadTexture(image,i++);
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
    if (tiempo<.01){
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
                        posXNotes=-450;
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
void playSound(){
    std::string path = "/Users/robertomtz/Desktop/Graficas/MIDIot/MIDIot/piano/";
    std::string cmd;
    cmd = "afplay -q 1 " + path + notaNombre[notaOprimidaActual] + ".wav & exit";
    system(cmd.c_str());
}
void dibuja()
{
    //puntos actuales jugador y dealer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLineWidth(2);
    glColor3f(0.0, 0.0, 0.0);
    
    if (entrar){
    glBindTexture(GL_TEXTURE_2D, texName[1]);
    glRectd(-470, 440, -380, 320);
    glRectd(-470, 265, -380, 145);
    
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
    
        if (start){
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
    init();
    glEnable(GL_DEPTH_TEST);
    glutKeyboardFunc(myKey);
    glutDisplayFunc(dibuja);
    glutTimerFunc(5, myTimer, 1);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}



#include <assert.h>
#include <fstream>

#include "imageloader.h"

using namespace std;

Image::Image(char* ps, int w, int h) : pixels(ps), width(w), height(h)
{
    
}

Image::~Image()
{
    delete[] pixels;
}

namespace
{
    //Converts a four-character array to an integer, using little-endian form
    int toInt(const char* bytes)
    {
        return (int)(((unsigned char)bytes[3] << 24) |
                     ((unsigned char)bytes[2] << 16) |
                     ((unsigned char)bytes[1] << 8) |
                     (unsigned char)bytes[0]);
    }
    
    //Converts a two-character array to a short, using little-endian form
    short toShort(const char* bytes)
    {
        return (short)(((unsigned char)bytes[1] << 8) |
                       (unsigned char)bytes[0]);
    }
    
    //Reads the next four bytes as an integer, using little-endian form
    int readInt(ifstream &input)
    {
        char buffer[4];
        input.read(buffer, 4);
        return toInt(buffer);
    }
    
    //Reads the next two bytes as a short, using little-endian form
    short readShort(ifstream &input)
    {
        char buffer[2];
        input.read(buffer, 2);
        return toShort(buffer);
    }
    
    //Just like auto_ptr, but for arrays
    template<class T>
    class auto_array
    {
    private:
        T* array;
        mutable bool isReleased;
    public:
        explicit auto_array(T* array_ = NULL) :
        array(array_), isReleased(false)
        {
        }
        
        auto_array(const auto_array<T> &aarray)
        {
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }
        
        ~auto_array()
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
        }
        
        T* get() const
        {
            return array;
        }
        
        T &operator*() const
        {
            return *array;
        }
        
        void operator=(const auto_array<T> &aarray)
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }
        
        T* operator->() const
        {
            return array;
        }
        
        T* release()
        {
            isReleased = true;
            return array;
        }
        
        void reset(T* array_ = NULL)
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
            array = array_;
        }
        
        T* operator+(int i)
        {
            return array + i;
        }
        
        T &operator[](int i)
        {
            return array[i];
        }
    };
}

Image* loadBMP(const char* filename)
{
    ifstream input;
    input.open(filename, ifstream::binary);
    assert(!input.fail() || !"Could not find file");
    char buffer[2];
    input.read(buffer, 2);
    assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Not a bitmap file");
    input.ignore(8);
    int dataOffset = readInt(input);
    
    //Read the header
    int headerSize = readInt(input);
    int width;
    int height;
    switch (headerSize)
    {
        case 40:
            //V3
            width = readInt(input);
            height = readInt(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
            assert(readShort(input) == 0 || !"Image is compressed");
            break;
        case 12:
            //OS/2 V1
            width = readShort(input);
            height = readShort(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
            break;
        case 64:
            //OS/2 V2
            assert(!"Can't load OS/2 V2 bitmaps");
            break;
        case 108:
            //Windows V4
            assert(!"Can't load Windows V4 bitmaps");
            break;
        case 124:
            //Windows V5
            assert(!"Can't load Windows V5 bitmaps");
            break;
        default:
            assert(!"Unknown bitmap format");
    }
    
    //Read the data
    int bytesPerRow = ((width * 3 + 3) / 4) * 4 - (width * 3 % 4);
    int size = bytesPerRow * height;
    auto_array<char> pixels(new char[size]);
    input.seekg(dataOffset, ios_base::beg);
    input.read(pixels.get(), size);
    
    //Get the data into the right format
    auto_array<char> pixels2(new char[width * height * 3]);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int c = 0; c < 3; c++)
            {
                pixels2[3 * (width * y + x) + c] =
                pixels[bytesPerRow * y + 3 * x + (2 - c)];
            }
        }
    }
    
    input.close();
    return new Image(pixels2.release(), width, height);
}

