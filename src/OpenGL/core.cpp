#include <Windows.h>
#include <cfloat>
#include "Context.h"
/*
	1. No error checking is done for any function yet.
	2. Currently handling 2D vertices only , needs 3D extension.
	3. Add glOrtho function.
	4. Add glPopMatrix function - Done 
	5. Add LoadMatrix 
	6. Add glTranslate, glScale, glRotate functions.
	7. Add glMultMatrix 
	8. glGet function to query current values 
	9. Add depth related functions. (glEnable, glDepthFunc, glDepthMask, glDepthRange)
	10. Seperate Vertex processing and Fragment Processing in two differnt function calls.
	11. Add support for 3D vert, Add depth handling in rasterization and Render cube.(This will utilize all matrix functions) 

*/
using namespace TinyGl;

void glBegin(GLenum mode)
{
	auto& ctx = Context::GetContext();
	ctx.SyncCurrentMatrix();
	ctx.m_capturedMV   = ctx.m_mvMatrixStack.top();
	ctx.m_capturedProj = ctx.m_projMatStack.top();
	ctx.m_isWithinBeginEnd = true;
	ctx.m_topology = mode;
}

void glClear(GLbitfield mask)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}

	auto& ctx = Context::GetContext();
	size_t sz = static_cast<size_t>(ctx.m_viewport.m_width) * ctx.m_viewport.m_height;

	if (mask & GL_COLOR_BUFFER_BIT)
		ctx.m_framebuffer.assign(sz, { 0.0f, 0.0f, 0.0f });
	if (mask & GL_DEPTH_BUFFER_BIT)
		ctx.m_depthBuffer.assign(sz, FLT_MAX);
	if (mask & GL_ACCUM_BUFFER_BIT)
		ctx.m_accumBuffer.clear();
	if (mask & GL_STENCIL_BUFFER_BIT)
		ctx.m_stencilBuffer.clear();

	if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
		ctx.m_glError = GL_INVALID_VALUE;
}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	Context::GetContext().m_currentColor = { red, green, blue };
}

void glEnd(void)
{
	//ToDo : generate a shape and store all prim under it
	if (Context::GetContext().m_topology == GL_TRIANGLES)
	{
		if (!Context::GetContext().m_vtxBuffer2D.empty())
		{
			Context::GetContext().m_nPrims = Context::GetContext().m_vtxBuffer2D.size() / 3;
			for (size_t i = 0; i < Context::GetContext().m_nPrims; i++)
			{
				Triangle<Vector2> t;
				t.v0 = Context::GetContext().m_vtxBuffer2D[i + 0];
				t.v1 = Context::GetContext().m_vtxBuffer2D[i + 1];
				t.v2 = Context::GetContext().m_vtxBuffer2D[i + 2];
				Context::GetContext().m_triangles2D.push_back(t);
			}
		}

		if (!Context::GetContext().m_vtxBuffer3D.empty())
		{
			Context::GetContext().m_nPrims = Context::GetContext().m_vtxBuffer3D.size() / 3;
			for (size_t i = 0; i < Context::GetContext().m_nPrims; i++)
			{
				Triangle<Vector3> t;
				t.v0 = Context::GetContext().m_vtxBuffer3D[i + 0];
				t.v1 = Context::GetContext().m_vtxBuffer3D[i + 1];
				t.v2 = Context::GetContext().m_vtxBuffer3D[i + 2];
				Context::GetContext().m_triangles3D.push_back(t);
			}
		}
	}
	Context::GetContext().m_isWithinBeginEnd = false;
}

void glFinish(void)
{
    Context::GetContext().SyncCurrentMatrix();
    Context::GetContext().TransformVertices();
    Context::GetContext().Rasterize();
}

void glFlush(void)
{
    Context::GetContext().SyncCurrentMatrix();
    Context::GetContext().TransformVertices();
    Context::GetContext().Rasterize();
}

GLenum glGetError(void)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		return GL_INVALID_OPERATION;
	}
	return Context::GetContext().m_glError;
}

void glVertex2f(GLfloat x, GLfloat y)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_vtxBuffer2D.push_back({ x, y });
	}
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_vtxBuffer3D.push_back({ x, y, z });
		Context::GetContext().m_colorBuffer.push_back(Context::GetContext().m_currentColor);
	}
}

void glVertex3i(GLint x, GLint y, GLint z)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_vtxBuffer3D.push_back({
			static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)
		});
		Context::GetContext().m_colorBuffer.push_back(Context::GetContext().m_currentColor);
	}
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	if (height < 0)
	{
		Context::GetContext().m_glError = GL_INVALID_VALUE;
		return;
	}

	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}

	Context::GetContext().m_viewport.m_x = x;
	Context::GetContext().m_viewport.m_y = y;
	Context::GetContext().m_viewport.m_width = width;
	Context::GetContext().m_viewport.m_height = height;
}

void glMatrixMode(GLenum mode)
{
	Context::GetContext().SyncCurrentMatrix();

	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}

	switch (mode)
	{
	case GL_MODELVIEW:
		Context::GetContext().m_currentMatrix = Context::GetContext().m_mvMatrixStack.top();
		Context::GetContext().m_currentMatStackType = Context::MatrixStackType::MatrixStackTypeModelView;

		break;
	case GL_PROJECTION:
		Context::GetContext().m_currentMatrix = Context::GetContext().m_projMatStack.top();
		Context::GetContext().m_currentMatStackType = Context::MatrixStackType::MatrixStackTypeProjection;
		break;
	case GL_TEXTURE:
		Context::GetContext().m_currentMatrix = Context::GetContext().m_textureMatStack.top();
		Context::GetContext().m_currentMatStackType = Context::MatrixStackType::MatrixStackTypeTexture;
		break;
	default:
		Context::GetContext().m_currentMatStackType = Context::MatrixStackType::MatrixStackTypeError;
		Context::GetContext().m_glError = GL_INVALID_ENUM;
		break;

	}
}

void glLoadIdentity(void)
{
	if (Context::GetContext().m_currentMatStackType != Context::MatrixStackType::MatrixStackTypeError)
	{
		Context::GetContext().m_currentMatrix = mt4x4Identity();
	}
}

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	if (zFar < 0.0f || zNear < 0.0f)
	{
		Context::GetContext().m_glError = GL_INVALID_ENUM;
		return;
	}
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}
	float A = static_cast<float>((right + left) / (right - left));
	float B = static_cast<float>((top + bottom) / (top - bottom));
	float C = static_cast<float>(-(zFar + zNear) / (zFar - zNear));
	float D = static_cast<float>(-(2 * zFar * zNear) / (zFar - zNear));
	float X = static_cast<float>((2 * zNear) / (right - left));
	float Y = static_cast<float>((2 * zNear) / (top - bottom));

	Matrix4x4 f;
	f.row[0] = { X,    0.0f,  A,  0.0f };
	f.row[1] = { 0.0f, Y   ,  B,  0.0f };
	f.row[2] = { 0.0f, 0.0f,  C,  D    };
	f.row[3] = { 0.0f, 0.0f, -1,  0.0f };

	//Note : Api currently assumes m_currentMat is set to proj properly. Need some error checking ,check out specs on this one.
	Context::GetContext().m_currentMatrix = matMultiply(Context::GetContext().m_currentMatrix, f);
}

void glPushMatrix(void)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}

	if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeModelView)
	{
		if (Context::GetContext().m_mvMatrixStack.size() > MaxModelViewMatStackDepth)
		{
			Context::GetContext().m_glError = GL_STACK_OVERFLOW;
			return;
		}
		Context::GetContext().m_mvMatrixStack.push(Context::GetContext().m_currentMatrix);
	}
	else if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeProjection)
	{
		if (Context::GetContext().m_projMatStack.size() > MaxProjMatStackDepth)
		{
			Context::GetContext().m_glError = GL_STACK_OVERFLOW;
			return;
		}
		Context::GetContext().m_projMatStack.push(Context::GetContext().m_currentMatrix);
	}
	else if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeTexture)
	{
		if (Context::GetContext().m_textureMatStack.size() > MaxTextureMatStackDepth)
		{
			Context::GetContext().m_glError = GL_STACK_OVERFLOW;
			return;
		}
		Context::GetContext().m_textureMatStack.push(Context::GetContext().m_currentMatrix);
	}
}

void glPopMatrix(void)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}

	if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeModelView)
	{
		if (Context::GetContext().m_mvMatrixStack.size() == 1)
		{
			Context::GetContext().m_glError = GL_STACK_UNDERFLOW;
			return;
		}
		Context::GetContext().m_currentMatrix = Context::GetContext().m_mvMatrixStack.top(); Context::GetContext().m_mvMatrixStack.pop();
	}
	else if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeProjection)
	{
		if (Context::GetContext().m_projMatStack.size() == 1)
		{
			Context::GetContext().m_glError = GL_STACK_UNDERFLOW;
			return;
		}
		Context::GetContext().m_currentMatrix = Context::GetContext().m_projMatStack.top(); Context::GetContext().m_projMatStack.pop();
	}
	else if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeTexture)
	{
		if (Context::GetContext().m_textureMatStack.size() == 1)
		{
			Context::GetContext().m_glError = GL_STACK_UNDERFLOW;
			return;
		}
		Context::GetContext().m_currentMatrix = Context::GetContext().m_textureMatStack.top(); Context::GetContext().m_textureMatStack.pop();
	}
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}
	
	if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeModelView ||
		Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeProjection)
	{
		Matrix4x4 t = mt4x4Translate({ x, y, z });
		Context::GetContext().m_currentMatrix = matMultiply(Context::GetContext().m_currentMatrix, t);
	}
}

void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}
	
	//ToDo : When scaling factor other than 1.0f is used with lighting enabled, we need to check if glEnable(GL_NORMALISE) is set
	if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeModelView ||
		Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeProjection)
	{
		Matrix4x4 t = mt4x4Scale({ x, y, z });
		Context::GetContext().m_currentMatrix = matMultiply(Context::GetContext().m_currentMatrix, t);
	}
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	if (Context::GetContext().m_isWithinBeginEnd)
	{
		Context::GetContext().m_glError = GL_INVALID_OPERATION;
		return;
	}
	
	if (Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeModelView ||
		Context::GetContext().m_currentMatStackType == Context::MatrixStackType::MatrixStackTypeProjection)
	{
		Matrix4x4 t = mt4x4Rotation({ angle, x, y, z });
		Context::GetContext().m_currentMatrix = matMultiply(Context::GetContext().m_currentMatrix, t);
	}
}
