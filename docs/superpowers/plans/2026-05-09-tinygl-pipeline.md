# TinyGL Pipeline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Wire up the full vertex transformation pipeline (MVP multiply → perspective divide → viewport map → rasterize → depth test) so the rotating pyramid test app produces a correct PNG output.

**Architecture:** Fix bugs in `Common.h`, `Core.cpp`, `Context.h`, and `Context.cpp` in sequence. Each task is independently buildable. The pipeline runs inside `glFlush()`/`glFinish()` and writes `output.png`. No new files are created.

**Tech Stack:** C++17, Visual Studio 2022, stb_image_write (already vendored), Windows (MSVC toolchain).

---

## File Map

| File | What changes |
|---|---|
| `src/OpenGL/Common.h` | Fix `mt4x4Rotation` (Rodrigues formula, uninitialized row[3]) |
| `src/OpenGL/Context.h` | Add `m_currentColor`, `m_screenBuffer`; declare `SyncCurrentMatrix()`, `TransformVertices()`; change `Rasterize()` signature |
| `src/OpenGL/Context.cpp` | Implement `TransformVertices()`, refactor `Rasterize()` |
| `src/OpenGL/Core.cpp` | Fix `glClear` bitmask, fix matrix mode default, add `SyncCurrentMatrix` call, fix `glColor3f`/`glVertex*` color latch, wire `TransformVertices` in `glFlush`/`glFinish` |
| `src/OpenGLTestApp/main.cpp` | Add viewport, projection matrix, and camera setup so the pyramid is visible |

---

## Task 1: Fix the rotation matrix in Common.h

**Files:**
- Modify: `src/OpenGL/Common.h:309-326`

The function `mt4x4Rotation` has two bugs:
1. `row[0]` columns 1 and 2 are identical (copy-paste error).
2. `row[3]` is never initialized — undefined behaviour.

- [ ] **Step 1: Replace `mt4x4Rotation` body**

In `src/OpenGL/Common.h`, replace the entire `mt4x4Rotation` function (lines 309–326) with:

```cpp
TGL_INLINE Matrix4x4 mt4x4Rotation(Vector4 r)
{
    float angle = r.x;
    Vector3 axis = { r.y, r.z, r.w };
    float angleRad = angle * (Pi / 180.0f);
    float c = std::cosf(angleRad);
    float s = std::sinf(angleRad);
    axis = normalize(axis);
    float t = 1.0f - c;
    float x = axis.x, y = axis.y, z = axis.z;

    Matrix4x4 rotation;
    rotation.row[0] = { c + t*x*x,     t*x*y - s*z,  t*x*z + s*y,  0.0f };
    rotation.row[1] = { t*x*y + s*z,   c + t*y*y,    t*y*z - s*x,  0.0f };
    rotation.row[2] = { t*x*z - s*y,   t*y*z + s*x,  c + t*z*z,    0.0f };
    rotation.row[3] = { 0.0f,          0.0f,          0.0f,          1.0f };
    return rotation;
}
```

- [ ] **Step 2: Build the OpenGL DLL project**

Open a Developer Command Prompt for VS 2022 and run:
```
msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64
```
Expected: Build succeeded, 0 errors.

- [ ] **Step 3: Commit**
```bash
git add src/OpenGL/Common.h
git commit -m "fix: correct Rodrigues rotation matrix formula and initialize row[3]"
```

---

## Task 2: Add new members and method declarations to Context.h

**Files:**
- Modify: `src/OpenGL/Context.h`

All subsequent tasks depend on these new members. Do this task first so every later task compiles independently.

- [ ] **Step 1: Add new members to Context.h**

In `Context.h`, inside the `public:` section, after the line `std::vector<Vector3> m_colorBuffer;`, add:

```cpp
Vector3 m_currentColor = { 1.0f, 1.0f, 1.0f }; // GL color latch, default white
std::vector<Vector3> m_framebuffer;              // pixel output (width * height)
std::vector<Vector3> m_screenBuffer;             // screen-space verts: x/y in pixels, z in NDC
```

- [ ] **Step 2: Add new method declarations to Context.h**

Replace the existing `void Rasterize();` declaration with:

```cpp
void SyncCurrentMatrix();
void TransformVertices();
void Rasterize();
```

- [ ] **Step 3: Build the DLL to confirm no compile errors**

```
msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64
```
Expected: Build succeeded, 0 errors. (`SyncCurrentMatrix` and `TransformVertices` are declared but not yet defined — this will produce linker errors if called, but the declaration alone is fine.)

- [ ] **Step 4: Commit**
```bash
git add src/OpenGL/Context.h
git commit -m "feat: add m_currentColor, m_framebuffer, m_screenBuffer and pipeline method declarations to Context"
```

---

## Task 3: Fix glClear to handle OR'd bitmask flags and init depth buffer correctly

**Files:**
- Modify: `src/OpenGL/Core.cpp:25-51`

`glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)` currently hits the `default` switch case and sets an error. Also, `GL_DEPTH_BUFFER_BIT` clears via `.clear()` (empties the vector) instead of filling it with `FLT_MAX`, so depth testing won't work.

- [ ] **Step 1: Add `#include <cfloat>` at the top of Core.cpp** (for `FLT_MAX`)

In `Core.cpp`, after `#include <Windows.h>` add:
```cpp
#include <cfloat>
```

- [ ] **Step 2: Replace the `glClear` function in Core.cpp**

Replace the entire `glClear` function with:

```cpp
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
```

- [ ] **Step 3: Build**
```
msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64
```
Expected: Build succeeded, 0 errors.

- [ ] **Step 4: Commit**
```bash
git add src/OpenGL/Core.cpp
git commit -m "fix: handle OR'd bitmask in glClear, init depth buffer to FLT_MAX"
```

---

## Task 4: Fix matrix mode default and add SyncCurrentMatrix

**Files:**
- Modify: `src/OpenGL/Context.h:70-75`
- Modify: `src/OpenGL/Context.cpp:15-30`
- Modify: `src/OpenGL/Core.cpp` (glMatrixMode function)

**Problem:** `m_currentMatStackType` defaults to `MatrixStackTypeError`, so `glTranslatef`, `glRotatef`, `glPushMatrix`, and `glPopMatrix` all silently do nothing. Also, when `glMatrixMode` switches stacks it discards the accumulated transforms in `m_currentMatrix` without saving them back.

- [ ] **Step 1: Change default matrix stack type in Context.h**

In `Context.h`, change:
```cpp
// Replace:
MatrixStackType m_currentMatStackType = MatrixStackType::MatrixStackTypeError;

// With:
MatrixStackType m_currentMatStackType = MatrixStackType::MatrixStackTypeModelView;
```

- [ ] **Step 2: Initialize `m_currentMatrix` to identity in Context constructor**

In `Context.cpp`, inside `Context::Context()`, after the `m_projMatStack.push(...)` line add:
```cpp
m_currentMatrix = mt4x4Identity();
```

- [ ] **Step 3: Implement `SyncCurrentMatrix` in Context.cpp**

Add this function after the `Context::~Context()` destructor:
```cpp
void Context::SyncCurrentMatrix()
{
    if (m_currentMatStackType == MatrixStackType::MatrixStackTypeModelView)
        m_mvMatrixStack.top() = m_currentMatrix;
    else if (m_currentMatStackType == MatrixStackType::MatrixStackTypeProjection)
        m_projMatStack.top() = m_currentMatrix;
    else if (m_currentMatStackType == MatrixStackType::MatrixStackTypeTexture)
        m_textureMatStack.top() = m_currentMatrix;
}
```

- [ ] **Step 4: Call `SyncCurrentMatrix()` at the start of `glMatrixMode`**

In `Core.cpp`, at the very start of `glMatrixMode`, before the existing `if (m_isWithinBeginEnd)` check, add:
```cpp
void glMatrixMode(GLenum mode)
{
    Context::GetContext().SyncCurrentMatrix();  // <-- add this line first

    if (Context::GetContext().m_isWithinBeginEnd)
    { ... }
    ...
}
```

- [ ] **Step 5: Build**
```
msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64
```
Expected: Build succeeded, 0 errors.

- [ ] **Step 6: Commit**
```bash
git add src/OpenGL/Context.h src/OpenGL/Context.cpp src/OpenGL/Core.cpp
git commit -m "fix: default matrix mode to ModelView, save matrix on mode switch"
```

---

## Task 5: Fix glColor3f / glVertex* color latch

**Files:**
- Modify: `src/OpenGL/Core.cpp` (glColor3f, glVertex3f, glVertex3i)

**Problem:** `glColor3f` pushes to `m_colorBuffer` immediately. But `glVertex3f` doesn't snapshot the current color — so colors accumulate independently of vertices. In GL 1.1, color is "current state" latched at vertex time. The new members (`m_currentColor`, `m_framebuffer`, `m_screenBuffer`) were added in Task 2.

- [ ] **Step 1: Fix `glColor3f` in Core.cpp**

Replace the entire `glColor3f` function:
```cpp
void glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    Context::GetContext().m_currentColor = { red, green, blue };
}
```

Note: the `m_isWithinBeginEnd` guard is removed — `glColor3f` is valid both inside and outside begin/end in GL 1.1. Setting current color outside a begin block is legal.

- [ ] **Step 3: Fix `glVertex3f` to snapshot current color**

Replace the entire `glVertex3f` function:
```cpp
void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (Context::GetContext().m_isWithinBeginEnd)
    {
        Context::GetContext().m_vtxBuffer3D.push_back({ x, y, z });
        Context::GetContext().m_colorBuffer.push_back(Context::GetContext().m_currentColor);
    }
}
```

- [ ] **Step 4: Fix `glVertex3i` to snapshot current color**

Replace the entire `glVertex3i` function:
```cpp
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
```

- [ ] **Step 5: Build**
```
msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64
```
Expected: Build succeeded, 0 errors.

- [ ] **Step 6: Commit**
```bash
git add src/OpenGL/Context.h src/OpenGL/Core.cpp
git commit -m "fix: glColor3f sets current color latch, glVertex snapshots it; add framebuffer/screenBuffer members"
```

---

## Task 6: Implement TransformVertices and wire into glFlush/glFinish

**Files:**
- Modify: `src/OpenGL/Context.cpp`
- Modify: `src/OpenGL/Core.cpp` (glFlush, glFinish)

This implements the three missing pipeline stages: MVP transform, perspective divide, and viewport transform. Results go into `m_screenBuffer` (one Vector3 per vertex: x/y in screen pixels, z in NDC [-1,1]).

- [ ] **Step 1: Implement `TransformVertices()` in Context.cpp**

Add this function after `SyncCurrentMatrix()`:

```cpp
void Context::TransformVertices()
{
    m_screenBuffer.clear();
    m_screenBuffer.reserve(m_vtxBuffer3D.size());

    Matrix4x4 mv   = m_mvMatrixStack.top();
    Matrix4x4 proj = m_projMatStack.top();

    float halfW = m_viewport.m_width  * 0.5f;
    float halfH = m_viewport.m_height * 0.5f;

    for (const Vector3& v : m_vtxBuffer3D)
    {
        // Stage 1: MVP transform → clip space
        Vector4 eye  = mulMatVec(mv,   { v.x, v.y, v.z, 1.0f });
        Vector4 clip = mulMatVec(proj, eye);

        // Stage 2: Perspective divide → NDC [-1, 1]
        float invW = 1.0f / clip.w;
        float ndcX = clip.x * invW;
        float ndcY = clip.y * invW;
        float ndcZ = clip.z * invW;

        // Stage 3: Viewport transform → screen pixels (Y flipped: NDC +Y = screen top)
        float sx = (ndcX + 1.0f) * halfW + m_viewport.m_x;
        float sy = (1.0f - ndcY) * halfH + m_viewport.m_y;

        m_screenBuffer.push_back({ sx, sy, ndcZ });
    }
}
```

- [ ] **Step 2: Wire `TransformVertices` into `glFlush` and `glFinish` in Core.cpp**

Replace both `glFlush` and `glFinish`:
```cpp
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
```

- [ ] **Step 3: Build**
```
msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64
```
Expected: Build succeeded, 0 errors.

- [ ] **Step 4: Commit**
```bash
git add src/OpenGL/Context.cpp src/OpenGL/Core.cpp
git commit -m "feat: implement TransformVertices (MVP, perspective divide, viewport map)"
```

---

## Task 7: Refactor Rasterize to use screen-space triangles and fix color interpolation

**Files:**
- Modify: `src/OpenGL/Context.cpp` (Rasterize function)

**Problems in current Rasterize:**
1. Uses `m_triangles2D` — which has no entries when using 3D vertices.
2. Sample coordinates are `{i*0.5f, j*0.5f}` — wrong scaling and wrong axis order.
3. Color interpolation reads `.x/.y/.z` from the **same** vertex for r/g/b — should read `.x` from each vertex for red, `.y` from each vertex for green, etc.
4. Writes to a local `pixels` array then to PNG — should write to `m_framebuffer` and use `m_depthBuffer` (done in Task 7).

- [ ] **Step 1: Replace the entire `Rasterize()` function in Context.cpp**

```cpp
void Context::Rasterize()
{
    assert(m_nPrims != 0);
    assert(m_viewport.m_width != 0 && m_viewport.m_height != 0);
    assert(m_screenBuffer.size() == m_vtxBuffer3D.size());

    int width  = m_viewport.m_width;
    int height = m_viewport.m_height;

    // Ensure framebuffer is allocated (glClear should have done this, but guard anyway)
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
        if (area <= 0.0f) continue;  // back-face or degenerate triangle

        // Bounding box clamped to viewport
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
                    w0 /= area;
                    w1 /= area;
                    w2 /= area;

                    // Interpolate depth
                    float z = w0 * m_screenBuffer[i0].z
                            + w1 * m_screenBuffer[i1].z
                            + w2 * m_screenBuffer[i2].z;

                    // Depth test
                    size_t pixId = row * width + col;
                    if (z < m_depthBuffer[pixId])
                    {
                        m_depthBuffer[pixId] = z;

                        // Interpolate color (each channel from the three vertices)
                        float r = w0 * m_colorBuffer[i0].x + w1 * m_colorBuffer[i1].x + w2 * m_colorBuffer[i2].x;
                        float g = w0 * m_colorBuffer[i0].y + w1 * m_colorBuffer[i1].y + w2 * m_colorBuffer[i2].y;
                        float b = w0 * m_colorBuffer[i0].z + w1 * m_colorBuffer[i1].z + w2 * m_colorBuffer[i2].z;

                        m_framebuffer[pixId] = { r, g, b };
                    }
                }
            }
        }
    }

    // Write framebuffer to PNG
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
```

- [ ] **Step 2: Add `#include <cfloat>` and `#include <algorithm>` to Context.cpp**

At the top of `Context.cpp`, add after `#include <cassert>`:
```cpp
#include <cfloat>
#include <algorithm>
```

- [ ] **Step 3: Build**
```
msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64
```
Expected: Build succeeded, 0 errors. The old `m_triangles2D`-based code is gone; `m_triangles2D` and `m_triangles3D` are now unused but can stay in Context.h.

- [ ] **Step 4: Commit**
```bash
git add src/OpenGL/Context.cpp
git commit -m "refactor: rasterize from screen-space verts, fix color interpolation, add depth test"
```

---

## Task 8: Update main.cpp with proper GL state setup

**Files:**
- Modify: `src/OpenGLTestApp/main.cpp`

**Problem:** The test app never calls `glViewport`, `glMatrixMode`, or sets up a projection matrix. With viewport at (0,0,0,0) the `assert` in `Rasterize` fires. The object is at the origin with no projection, so nothing would be visible anyway.

- [ ] **Step 1: Replace `main.cpp` with a version that sets up proper GL state**

```cpp
#include <Windows.h>
#include <gl.h>

static void display()
{
    // Set up viewport and projection once
    glViewport(0, 0, 800, 600);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Perspective frustum: 60-degree-ish FOV, 4:3 aspect, near=1, far=100
    glFrustum(-0.75, 0.75, -0.5625, 0.5625, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -5.0f);   // move pyramid 5 units in front of camera
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f); // tilt down 30 degrees to see top face
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f); // rotate 45 degrees around Y

    glBegin(GL_TRIANGLES);

#define TOP glColor3f(1.0f, 0.0f, 0.0f); glVertex3i(0, 1, 0)
#define FR  glColor3f(0.0f, 1.0f, 0.0f); glVertex3i(1, -1, 1)
#define FL  glColor3f(0.0f, 0.0f, 1.0f); glVertex3i(-1, -1, 1)
#define BR  glColor3f(0.0f, 0.0f, 1.0f); glVertex3i(1, -1, -1)
#define BL  glColor3f(0.0f, 1.0f, 0.0f); glVertex3i(-1, -1, -1)

    TOP; FL; FR;
    TOP; FR; BR;
    TOP; BR; BL;
    TOP; BL; FL;
    FR; FL; BL;
    BL; BR; FR;

    glEnd();
    glPopMatrix();
    glFlush();
}

int main()
{
    display();
    return 0;
}
```

- [ ] **Step 2: Build both projects**

```
msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64
msbuild src\OpenGLTestApp\OpenGLTestApp.vcxproj /p:Configuration=Debug /p:Platform=x64
```
Expected: Both build with 0 errors.

- [ ] **Step 3: Run the test app**

Run `OpenGLTestApp.exe` from the output directory. Expected: it exits immediately (no window) and writes `output.png` in the working directory.

- [ ] **Step 4: Inspect output.png**

Open `output.png` (800×600). Expected:
- Black background
- A colourful pyramid shape visible near the center
- Red top vertex, green and blue base vertices with interpolated colours across faces
- Back faces correctly occluded by front faces (depth testing working)

If the image is all black, see the debugging checklist below.

- [ ] **Step 5: Commit**
```bash
git add src/OpenGLTestApp/main.cpp
git commit -m "feat: add proper viewport, projection, and camera setup to test app"
```

---

## Debugging Checklist (if output.png is all black)

1. **Assert fired?** Run in Debug mode in VS and check if any `assert()` fires — the most likely ones are in `Rasterize()`.
2. **No triangles rasterized?** Add `assert(m_nPrims > 0);` at the top of `Rasterize()` to confirm `glEnd` is populating the buffers.
3. **All triangles back-facing?** The `area <= 0` skip removes back faces. Add a temporary `if (area == 0.0f) continue;` (removing the `< 0` part) to disable back-face culling and see if anything appears.
4. **Projection clips everything?** Temporarily replace `glFrustum` with `glLoadIdentity()` and render with vertices already in NDC range (e.g. `glVertex3f(0, 0.5, -0.5)`). If that renders, the issue is in the frustum parameters or z-range.
5. **Viewport not set?** `m_viewport.m_width` must be 800 before `glFlush`. Add `assert(Context::GetContext().m_viewport.m_width == 800)` in `glFlush` to verify.
