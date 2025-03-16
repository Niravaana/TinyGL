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
		HDC m_hdc; //hold device context for current thread
		GLenum m_topology;
		Viewport m_viewport;
		GLenum m_glError;
		GLuint m_nPrims;
		std::vector<Vector2> m_vtxBuffer;
		std::vector<Vector2> m_idxBuffer;
		std::vector<Vector3> m_colorBuffer;
		std::vector<Triangle<Vector2>> m_triangles2D;
		std::vector<Triangle<Vector3>> m_triangles3D;

		Matrix4x4 m_currentModelViewMatrix; // point to top of the stack matrix( we might not need this copy ??)
		Matrix4x4 m_currentProjStack;
		Matrix4x4 m_currentTextureStack;
		std::stack<Matrix4x4> m_mvMatrixStack;
		std::stack<Matrix4x4> m_projMatStack;
		std::stack<Matrix4x4> m_textureMatStack;
	};
}