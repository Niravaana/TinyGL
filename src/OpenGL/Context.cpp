#define NOMINMAX
#include "Context.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbi_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cassert>
#include <cfloat>
#include <algorithm>

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
	m_currentMatrix = mt4x4Identity();

	//ToDo : Clear depth values with approp set value
	m_depthBuffer.resize(m_viewport.m_width * m_viewport.m_height);
	m_stencilBuffer.resize(m_viewport.m_width * m_viewport.m_height);
	m_accumBuffer.resize(m_viewport.m_width * m_viewport.m_height);
}

Context::~Context()
{

}

void Context::SyncCurrentMatrix()
{
    if (m_currentMatStackType == MatrixStackType::MatrixStackTypeModelView)
        m_mvMatrixStack.top() = m_currentMatrix;
    else if (m_currentMatStackType == MatrixStackType::MatrixStackTypeProjection)
        m_projMatStack.top() = m_currentMatrix;
    else if (m_currentMatStackType == MatrixStackType::MatrixStackTypeTexture)
        m_textureMatStack.top() = m_currentMatrix;
}

void Context::TransformVertices()
{
    m_screenBuffer.clear();
    m_screenBuffer.reserve(m_vtxBuffer3D.size());

    Matrix4x4 mv   = m_capturedMV;
    Matrix4x4 proj = m_capturedProj;

    float halfW = m_viewport.m_width  * 0.5f;
    float halfH = m_viewport.m_height * 0.5f;

    for (const Vector3& v : m_vtxBuffer3D)
    {
        Vector4 eye  = mulMatVec(mv,   { v.x, v.y, v.z, 1.0f });
        Vector4 clip = mulMatVec(proj, eye);

        float invW = 1.0f / clip.w;
        float ndcX = clip.x * invW;
        float ndcY = clip.y * invW;
        float ndcZ = clip.z * invW;

        float sx = (ndcX + 1.0f) * halfW + m_viewport.m_x;
        float sy = (1.0f - ndcY) * halfH + m_viewport.m_y;

        m_screenBuffer.push_back({ sx, sy, ndcZ });
    }
}

void Context::Rasterize()
{
    assert(m_nPrims != 0);
    assert(m_viewport.m_width != 0 && m_viewport.m_height != 0);
    assert(m_screenBuffer.size() == m_vtxBuffer3D.size());

    int width  = m_viewport.m_width;
    int height = m_viewport.m_height;

    if (m_framebuffer.size() != static_cast<size_t>(width * height))
        m_framebuffer.assign(width * height, { 0.0f, 0.0f, 0.0f });
    if (m_depthBuffer.size() != static_cast<size_t>(width * height))
        m_depthBuffer.assign(width * height, FLT_MAX);

    for (size_t primId = 0; primId < m_nPrims; primId++)
    {
        size_t i0 = primId * 3 + 0;
        size_t i1 = primId * 3 + 1;
        size_t i2 = primId * 3 + 2;

        Vector2 p0 = { m_screenBuffer[i0].x, m_screenBuffer[i0].y };
        Vector2 p1 = { m_screenBuffer[i1].x, m_screenBuffer[i1].y };
        Vector2 p2 = { m_screenBuffer[i2].x, m_screenBuffer[i2].y };

        float area = edgeFunction(p0, p1, p2);
        if (area <= 0.0f) continue;

        int minX = static_cast<int>(std::max(0.0f, std::min({ p0.x, p1.x, p2.x })));
        int minY = static_cast<int>(std::max(0.0f, std::min({ p0.y, p1.y, p2.y })));
        int maxX = static_cast<int>(std::min((float)(width  - 1), std::max({ p0.x, p1.x, p2.x })));
        int maxY = static_cast<int>(std::min((float)(height - 1), std::max({ p0.y, p1.y, p2.y })));

        for (int row = minY; row <= maxY; row++)
        {
            for (int col = minX; col <= maxX; col++)
            {
                Vector2 sample = { col + 0.5f, row + 0.5f };

                float w0 = edgeFunction(p1, p2, sample);
                float w1 = edgeFunction(p2, p0, sample);
                float w2 = edgeFunction(p0, p1, sample);

                if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
                {
                    w0 /= area; w1 /= area; w2 /= area;

                    float z = w0 * m_screenBuffer[i0].z
                            + w1 * m_screenBuffer[i1].z
                            + w2 * m_screenBuffer[i2].z;

                    size_t pixId = row * width + col;
                    if (z < m_depthBuffer[pixId])
                    {
                        m_depthBuffer[pixId] = z;

                        float r = w0 * m_colorBuffer[i0].x + w1 * m_colorBuffer[i1].x + w2 * m_colorBuffer[i2].x;
                        float g = w0 * m_colorBuffer[i0].y + w1 * m_colorBuffer[i1].y + w2 * m_colorBuffer[i2].y;
                        float b = w0 * m_colorBuffer[i0].z + w1 * m_colorBuffer[i1].z + w2 * m_colorBuffer[i2].z;

                        m_framebuffer[pixId] = { r, g, b };
                    }
                }
            }
        }
    }

    std::vector<uint8_t> pixels(width * height * 4);
    for (int i = 0; i < width * height; i++)
    {
        pixels[i * 4 + 0] = static_cast<uint8_t>(std::min(m_framebuffer[i].x, 1.0f) * 255);
        pixels[i * 4 + 1] = static_cast<uint8_t>(std::min(m_framebuffer[i].y, 1.0f) * 255);
        pixels[i * 4 + 2] = static_cast<uint8_t>(std::min(m_framebuffer[i].z, 1.0f) * 255);
        pixels[i * 4 + 3] = 255;
    }
    stbi_write_png("output.png", width, height, 4, pixels.data(), width * 4);
}
