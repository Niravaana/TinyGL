#include<Windows.h>
#include<gl.h>

GLfloat trans[3];			/* current translation */
GLfloat rot[2];				/* current rotation */

enum { 
    PAN = 1,				/* pan state bit */
    ROTATE,				/* rotate state bits */
    ZOOM				/* zoom state bit */
};

static void update(int state, int ox, int nx, int oy, int ny)
{
    int dx = ox - nx;
    int dy = ny - oy;

    switch(state) {
    case PAN:
	trans[0] -= dx / 100.0f;
	trans[1] -= dy / 100.0f;
	break;
    case ROTATE:
	rot[0] += (dy * 180.0f) / 500.0f;
	rot[1] -= (dx * 180.0f) / 500.0f;
#define clamp(x) x = x > 360.0f ? x-360.0f : x < -360.0f ? x+=360.0f : x
	clamp(rot[0]);
	clamp(rot[1]);
	break;
    case ZOOM:
	trans[2] -= (dx+dy) / 100.0f;
	break;
    }
}

static void display()
{
    glViewport(0, 0, 800, 600);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.75, 0.75, -0.5625, 0.5625, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -5.0f);   // move pyramid in front of camera
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLES);

#define TOP glColor3f(1.0f, 0.0f, 0.0f); glVertex3i(0, 1, 0)
#define FR  glColor3f(0.0f, 1.0f, 0.0f); glVertex3i(1, -1, 1)
#define FL  glColor3f(0.0f, 0.0f, 1.0f); glVertex3i(-1, -1, 1)
#define BR  glColor3f(0.0f, 0.0f, 1.0f); glVertex3i(1, -1, -1)
#define BL  glColor3f(0.0f, 1.0f, 0.0f); glVertex3i(-1, -1, -1)

    TOP; FL; FR;
    TOP; FR; BR;
    TOP; BR; BL;
    TOP; BL; FL;
    FR; FL; BL;
    BL; BR; FR;
    
    glEnd();
    glPopMatrix();
    glFlush();
}

int main()
{
    display();
    return 0;
}