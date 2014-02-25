/*-----------------------------------------------------------------------------
/ openGL playground
/------------------------------------------------------------------------------
/ ihsan Kehribar - 2014
/----------------------------------------------------------------------------*/
#include <stdlib.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdint.h>
#include <termios.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "globalDefinitions.h"

/* Main display buffer */
uint16_t screenBuffer[WIDTH];
extern void fillScreenBuffer(uint16_t* screenBufferPointer,uint16_t size,uint16_t topValue);
extern int platform_init(int argc, char** argv);

/* On-screen FPS counter print */
char fpsBuff[128];
#define FPS_PRINT 1

/* Cursor variables */
uint16_t cursor1_x;
uint16_t cursor1_y;
uint16_t cursor2_x;
uint16_t cursor2_y;
uint16_t cursor1_enable;
uint16_t cursor2_enable;

/* run/pause variable */
uint8_t pause_acquisition = 0;

/*-----------------------------------------------------------------------------
/ FPS calculation variables
/----------------------------------------------------------------------------*/
struct timeval ct;
uint32_t last_sec = 0;
uint32_t last_usec = 0;
uint32_t current_sec = 0;
uint32_t current_usec = 0;
#define interv_memory_size 100
float interv_memory[interv_memory_size];
/*---------------------------------------------------------------------------*/
void calculate_print_FPS();
void keyb(unsigned char key, int x, int y);
void myMouseFunc( int button, int state, int x, int y );
void glutPrint(float x, float y, char* text, float r, float g, float b, float a);
/*---------------------------------------------------------------------------*/
static void idle_function(void)
{          
    /* Calculate the FPS */
    calculate_print_FPS();

    /* Fill the screen draw buffer */
    fillScreenBuffer(screenBuffer,WIDTH,HEIGHT);

    /* Call the draw function */
    glutPostRedisplay();
}
/*---------------------------------------------------------------------------*/
static void display_function(void)
{
    glClear(GL_COLOR_BUFFER_BIT);    

    glMatrixMode(GL_MODELVIEW);           

    glLoadIdentity();       

    if(cursor1_enable)
    {
        glPushAttrib(GL_ENABLE_BIT); 
            glLineStipple(1, 0xAAAA);  
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINES);            
                
                // color of the line (r,g,b)
                glColor3f(0.27,0.35,0.45);

                // start point (x,y)
                glVertex2f(cursor1_x,0);

                // end point (x,y)
                glVertex2f(cursor1_x,HEIGHT);

                // color of the line (r,g,b)
                glColor3f(0.27,0.35,0.45);

                // start point (x,y)
                glVertex2f(0,cursor1_y);

                // end point (x,y)            
                glVertex2f(WIDTH,cursor1_y);

            glEnd();
        glPopAttrib();    
    }

    if(cursor2_enable)
    {
        glPushAttrib(GL_ENABLE_BIT); 
            glLineStipple(1, 0xAAAA);  
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINES);            
                
                // color of the line (r,g,b)
                glColor3f(0.45,0.27,0.35);

                // start point (x,y)
                glVertex2f(cursor2_x,0);

                // end point (x,y)
                glVertex2f(cursor2_x,HEIGHT);

                // color of the line (r,g,b)
                glColor3f(0.45,0.27,0.35);

                // start point (x,y)
                glVertex2f(0,cursor2_y);

                // end point (x,y)            
                glVertex2f(WIDTH,cursor2_y);

            glEnd();
        glPopAttrib();    
    }

    glBegin(GL_LINES);    

        /* Draw the screne buffer */    
        for (int i = 0; i < WIDTH; i++)
        {
            // color of the line (r,g,b)
            glColor3f (0.75,0.75,0.75);       

            // start point (x,y)
            glVertex2f(i,screenBuffer[i]);

            // end point (x,y)
            glVertex2f(i+1,screenBuffer[i]);
        } 

    glEnd();
    
    /* Draw the 'middle point' line */
    glPushAttrib(GL_ENABLE_BIT); 
        glLineStipple(3, 0xAAAA);  
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINES);
        glColor3f (0.7,0.3,0);  
        glVertex2f(0,HEIGHT/2);
        glVertex2f(WIDTH,HEIGHT/2);
        glEnd();
    glPopAttrib();
    
    #if FPS_PRINT
        glutPrint(2,HEIGHT-10,fpsBuff,0.75,0.75,0.75,1);
    #endif

    glutSwapBuffers();
}
/*---------------------------------------------------------------------------*/
void myinit(void)
{
    // initial background color
    glClearColor(0.26, 0.26, 0.26, 0.0);
    
    glMatrixMode(GL_PROJECTION);
    
    glLoadIdentity();

    // defining the corner points of the window
    gluOrtho2D( 0, WIDTH, 0, HEIGHT); 
    
    glMatrixMode(GL_MODELVIEW);       

    #if 0
        // taken from: http://www.glprogramming.com/red/chapter06.html    
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH_HINT,GL_DONT_CARE);
    #endif

    glLineWidth(1);
}
/*-----------------------------------------------------------------------------
/ MAIN ENTRY FUNCTION 
/----------------------------------------------------------------------------*/
int main(int argc, char** argv)
{  
    if(platform_init(argc,argv) != 0)
    {
        printf("[Err:] Platform init. problem!\n");
        exit(0);
    }

    glutInit(&argc, argv);
    
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    
    glutInitWindowSize(WIDTH,HEIGHT);
    
    glutCreateWindow("Real time plotter");
    
    // callback functions
    glutKeyboardFunc(&keyb);
    glutMouseFunc(&myMouseFunc);    
    glutIdleFunc(&idle_function);    
    glutDisplayFunc(&display_function);    
    
    glEnable(GL_BLEND);

    myinit();    

    glutMainLoop();

    return 0;
}
/*---------------------------------------------------------------------------*/
void calculate_print_FPS()
{
    float interval = 0;
    float interv_avrg = 0;
    
    gettimeofday(&ct, NULL);

    last_usec = current_usec;
    current_usec = ct.tv_usec;

    last_sec = current_sec;
    current_sec = ct.tv_sec;

    if(last_sec == current_sec)
    {
        interval = (current_usec-last_usec)/1000.0;
        #if 0
            printf("\n\n");
            printf("%f msec interval\n",interval);
            printf("%f instant fps\n",1000.0/interval);        
        #endif
    }
    else
    {
        interval = (((1000 * 1000) + current_usec) - last_usec)/1000.0;
        #if 0
            printf("\n\n");
            printf("%f msec interval\n",interval);
            printf("%f instant fps\n",1000.0/interval);
        #endif
    }

    for(int i=1;i<interv_memory_size;i++)
    {
        interv_memory[interv_memory_size-i] = interv_memory[interv_memory_size-1-i];
    }

    interv_memory[0] = interval;

    for(int i=0;i<interv_memory_size;i++)
    {     
        interv_avrg += interv_memory[i];
    }   
    
    interv_avrg /= (double)interv_memory_size;

    sprintf(fpsBuff,"Average fps: %3.3f",1000.0/interv_avrg);    
}
/*---------------------------------------------------------------------------*/
void glutPrint(float x, float y, char* text, float r, float g, float b, float a)
{ 
    if(!text || !strlen(text)) return; 
    bool blending = false; 
    if(glIsEnabled(GL_BLEND)) blending = true; 
    glEnable(GL_BLEND); 
    glColor4f(r,g,b,a); 
    glRasterPos2f(x,y); 
    while (*text) 
    { 
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *text); 
        text++; 
    } 
    if(!blending) glDisable(GL_BLEND); 
}  
/*---------------------------------------------------------------------------*/
void myMouseFunc( int button, int state, int x, int y ) 
{
    printf("button: %d state: %d x_pos: %4d y_pos: %4d\n",button,state,x,y);
    
    if(button == 0)
    {
        cursor1_enable = 1;
        cursor1_x = x;
        cursor1_y = HEIGHT - y;
    }

    if(button == 2)
    {
        cursor2_enable = 1;
        cursor2_x = x;
        cursor2_y = HEIGHT - y;
    }

}
/*---------------------------------------------------------------------------*/
void keyb(unsigned char key, int x, int y)
{
    printf("key: %3d x_pos: %4d y_pos: %4d\n",key,x,y);

    if(key == 'q')
    {
        printf("Bye!\n");
        exit(0);
    }

    if(key == 'x')
    {
        cursor1_enable = 0;
        cursor2_enable = 0;
    }

    if(key == 32)
    {
        pause_acquisition = !pause_acquisition;
    }
}
/*---------------------------------------------------------------------------*/
