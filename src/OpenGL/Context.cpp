#include "Context.h"
#include <cassert>

inline float edgeFunction(const float2 &a, const float2 &b, const float2 &c) {
    return (c.m_x - a.m_x) * (b.m_y - a.m_y) - (c.m_y - a.m_y) * (b.m_x - a.m_x);
}


Context::Context()
{
	m_hdc = wglGetCurrentDC();
	m_topology = GL_TRIANGLES;
	m_viewport.m_x = m_viewport.m_y = m_viewport.m_width = m_viewport.m_height = 0;
	m_glError = GL_NO_ERROR;
	m_nPrims = 0;
}

Context::~Context()
{

}

void Context::Rasterize()
{
	assert(m_nPrims != 0);
	assert(m_viewport.m_width != 0 && m_viewport.m_height != 0);

	for (size_t primId = 0; primId < m_nPrims; primId++)
	{
		for (size_t i = 0; i < m_viewport.m_height; i++)
		{
			for (size_t j = 0; j < m_viewport.m_width; j++)
			{
				float2 sample = { i * 0.5f, j * 0.5f };

			}
		}
	}
}
