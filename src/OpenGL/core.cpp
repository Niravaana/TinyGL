#include <Windows.h>
#include "Context.h"
/*
	No error checking is done for any function yet.
*/

void glBegin(GLenum mode)
{
	Context::GetContext().m_topology = mode;
}

void glClear(GLbitfield mask)
{

}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	Context::GetContext().m_colorBuffer.push_back({ red, green, blue });
}

void glEnd(void)
{
	//ToDo : generate a shape and store all prim under it
	if (Context::GetContext().m_topology == GL_TRIANGLES)
	{
		Context::GetContext().m_nPrims = Context::GetContext().m_vtxBuffer.size() / 3;
	}
}

void glFinish(void)
{
}

void glFlush(void)
{

}

GLenum glGetError(void)
{
	return Context::GetContext().m_glError;
}

void glVertex2f(GLfloat x, GLfloat y)
{
	Context::GetContext().m_vtxBuffer.push_back({x, y});
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	Context::GetContext().m_viewport.m_x = x;
	Context::GetContext().m_viewport.m_y = y;
	Context::GetContext().m_viewport.m_width = width;
	Context::GetContext().m_viewport.m_height = height;
}
