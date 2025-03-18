#include "Context.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbi_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cassert>

using namespace TinyGl;

inline float edgeFunction(const Vector2 &a, const Vector2 &b, const Vector2 &c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}


Context::Context()
{
	//m_hdc = GetDC();
	m_topology = GL_TRIANGLES;
	m_viewport.m_x = m_viewport.m_y = m_viewport.m_width = m_viewport.m_height = 0;
	m_glError = GL_NO_ERROR;
	m_nPrims = 0;
	m_mvMatrixStack.push(mt4x4Identity());
	m_projMatStack.push(mt4x4Identity());
	m_textureMatStack.push(mt4x4Identity());

	//ToDo : Clear depth values with approp set value
	m_depthBuffer.resize(m_viewport.m_width * m_viewport.m_height);
	m_stencilBuffer.resize(m_viewport.m_width * m_viewport.m_height);
	m_accumBuffer.resize(m_viewport.m_width * m_viewport.m_height);
}

Context::~Context()
{

}

void Context::Rasterize()
{
	assert(m_nPrims != 0);
	assert(m_viewport.m_width != 0 && m_viewport.m_height != 0);
	std::vector<uint8_t> pixels( m_viewport.m_width * m_viewport.m_height * 4 );

	for (size_t primId = 0; primId < m_nPrims; primId++)
	{
		float area = edgeFunction(m_triangles2D[primId].v0, m_triangles2D[primId].v1, m_triangles2D[primId].v2);
		for (size_t i = 0; i < m_viewport.m_height; i++)
		{
			for (size_t j = 0; j < m_viewport.m_width; j++)
			{
				Vector2 sample = { i * 0.5f, j * 0.5f };
				
				float w0 = edgeFunction(m_triangles2D[primId].v1, m_triangles2D[primId].v2, sample);
				float w1 = edgeFunction(m_triangles2D[primId].v2, m_triangles2D[primId].v0, sample);
				float w2 = edgeFunction(m_triangles2D[primId].v0, m_triangles2D[primId].v1, sample);

				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					w0 /= area;
					w1 /= area;
					w2 /= area;

					float r = w0 * m_colorBuffer[primId * 3 + 0].x + w1 * m_colorBuffer[primId * 3 + 0].y + w2 * m_colorBuffer[primId * 3 + 0].z;
					float g = w0 * m_colorBuffer[primId * 3 + 1].x + w1 * m_colorBuffer[primId * 3 + 1].y + w2 * m_colorBuffer[primId * 3 + 1].z;
					float b = w0 * m_colorBuffer[primId * 3 + 2].x + w1 * m_colorBuffer[primId * 3 + 2].y + w2 * m_colorBuffer[primId * 3 + 2].z;

					int pixId = j + i * m_viewport.m_width;
					pixels[pixId * 4 + 0] = static_cast<uint8_t>(r * 255);
					pixels[pixId * 4 + 1] = static_cast<uint8_t>(g * 255);
					pixels[pixId * 4 + 2] = static_cast<uint8_t>(b * 255);
					pixels[pixId * 4 + 3] = 255;
				}
			}
		}
	}

	stbi_write_png( "output.png", m_viewport.m_width, m_viewport.m_height, 4, pixels.data(), m_viewport.m_width * 4);
}
