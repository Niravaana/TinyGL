#pragma once
#include <Windows.h>
#include <vector>
#include <stack>
#include "gl.h"
#include "Common.h"
/*
ToDo 
Currently I am assuming only main thread creates a opengl context and only one contex is live.
*/

namespace TinyGl
{
	constexpr u32 MaxProjMatStackDepth = 2;
	constexpr u32 MaxModelViewMatStackDepth = 32;
	constexpr u32 MaxTextureMatStackDepth = 2;

	class Context
	{
	public:
		enum class MatrixStackType
		{
			MatrixStackTypeModelView,
			MatrixStackTypeProjection,
			MatrixStackTypeTexture,
			MatrixStackTypeError
		};

		struct Viewport
		{
			GLint m_x;
			GLint m_y;
			GLsizei m_width;
			GLsizei m_height;
		};

		Context();
		~Context();

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(Context&&) = delete;

		//Beyond c++11 static local variable is thread safe initialised
		static Context& GetContext()
		{
			static Context instance;
			return instance;
		}

		void Rasterize();

	public:
		HDC m_hdc = NULL; //hold device context for current thread
		GLenum m_topology = GL_TRIANGLES;
		Viewport m_viewport;
		GLenum m_glError;
		GLuint m_nPrims;
		std::vector<Vector2> m_vtxBuffer2D;
		std::vector<Vector2> m_idxBuffer2D;
		std::vector<Vector3> m_vtxBuffer3D;
		std::vector<Vector3> m_idxBuffer3D;
		std::vector<Vector3> m_colorBuffer;
		std::vector<GLfloat> m_depthBuffer;
		std::vector<GLfloat> m_accumBuffer;
		std::vector<GLfloat> m_stencilBuffer;
		std::vector<Triangle<Vector2>> m_triangles2D;
		std::vector<Triangle<Vector3>> m_triangles3D;

		MatrixStackType m_currentMatStackType = MatrixStackType::MatrixStackTypeError;
		Matrix4x4 m_currentMatrix;
		std::stack<Matrix4x4> m_mvMatrixStack;
		std::stack<Matrix4x4> m_projMatStack;
		std::stack<Matrix4x4> m_textureMatStack;
		bool m_isWithinBeginEnd = false;
	};
}