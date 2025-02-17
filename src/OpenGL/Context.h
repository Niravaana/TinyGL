#pragma once
#include <Windows.h>
#include <vector>
#include "gl.h"
#include "Common.h"
/*
ToDo 
Currently I am assuming only main thread creates a opengl context and only one contex is live.
*/

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
	std::vector<float2> m_vtxBuffer;
	std::vector<float2> m_idxBuffer;
	std::vector<float3> m_colorBuffer;
	std::vector<Triangle> m_triangles;
};