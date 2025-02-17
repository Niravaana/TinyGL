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
		float area = edgeFunction(m_triangles[primId].m_v0, m_triangles[primId].m_v1, m_triangles[primId].m_v2);
		for (size_t i = 0; i < m_viewport.m_height; i++)
		{
			for (size_t j = 0; j < m_viewport.m_width; j++)
			{
				float2 sample = { i * 0.5f, j * 0.5f };
				
				float w0 = edgeFunction(m_triangles[primId].m_v1, m_triangles[primId].m_v2, sample);
				float w1 = edgeFunction(m_triangles[primId].m_v2, m_triangles[primId].m_v0, sample);
				float w2 = edgeFunction(m_triangles[primId].m_v0, m_triangles[primId].m_v1, sample);

				if (w0 >= 0 && &w1 >= 0 && w2 >= 0)
				{
					w0 /= area;
					w1 /= area;
					w2 /= area;

					float r = w0 * m_colorBuffer[primId * 3 + 0].m_x + w1 * m_colorBuffer[primId * 3 + 0].m_y + w2 * m_colorBuffer[primId * 3 + 0].m_z;
					float g = w0 * m_colorBuffer[primId * 3 + 1].m_x + w1 * m_colorBuffer[primId * 3 + 1].m_y + w2 * m_colorBuffer[primId * 3 + 1].m_z;
					float b = w0 * m_colorBuffer[primId * 3 + 2].m_x + w1 * m_colorBuffer[primId * 3 + 2].m_y + w2 * m_colorBuffer[primId * 3 + 2].m_z;

					SetPixel(m_hdc, i, j, RGB(r * 255, g * 255, b * 255));
				}
			}
		}
	}
}
