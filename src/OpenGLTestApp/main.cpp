#include<Windows.h>
#include<gl.h>


void
display()
{
    glViewport(0, 0, 512,512);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(50.0f, 50.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(200.0f, 200.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(200.0f, 50.0f);
    glEnd();
    glFlush();
}

int main()
{
    display();
    return 0;
}