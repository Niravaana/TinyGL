# TinyGL Pipeline Design
**Date:** 2026-05-09  
**Goal:** Get the existing rotating pyramid test app (`OpenGLTestApp/main.cpp`) producing correct PNG output by wiring up the vertex transformation pipeline, fixing known bugs, and refactoring `Context.cpp` into clearly named pipeline stages.

---

## Scope

- **In scope:** Fix vertex transform pipeline, fix rotation matrix bug, add depth testing, refactor `Context.cpp` into named pipeline stages, fix rasterizer coordinate space, separate framebuffer from vertex color buffer, fix `glColor3f`/`glVertex*` interaction.
- **Out of scope:** Textures, lighting, `glOrtho`, additional primitive types, windowed output, blending, stencil operations.
- **Output target:** PNG file (via stb_image_write). No window system required.

---

## File Responsibilities

| File | Changes |
|---|---|
| `Common.h` | Fix rotation matrix formula (Rodrigues); add `Vector4` helpers if needed |
| `Context.h` | Add `m_framebuffer` (pixel color array); add `m_currentColor` (GL color latch); declare pipeline stage methods |
| `Context.cpp` | Refactor monolithic `Rasterize()` into five named stages; implement depth test |
| `Core.cpp` | Fix `glColor3f` to write to `m_currentColor`; fix `glVertex*` to snapshot `m_currentColor` into per-vertex color buffer |
| `gl.h` | No changes |

---

## Pipeline Stages

Executed in order inside `glFlush()` / `glFinish()`:

```
glEnd()
  └─ Assembles triangles from vertex buffer (already works)

glFlush() / glFinish()
  1. TransformVertices()   — v_clip = Projection * ModelView * vec4(v.xyz, 1.0)
  2. PerspectiveDivide()   — v_ndc = v_clip.xyz / v_clip.w
  3. ViewportTransform()   — map NDC [-1,1] to screen pixel coordinates
  4. RasterizeTriangle()   — edge function coverage + barycentric color interpolation
  5. RunFragmentTests()    — depth test; write color + depth if passed
  6. WriteOutput()         — stb_image_write to output.png
```

### Stage 1: TransformVertices
For each vertex `v` in `m_vtxBuffer3D`:
```
Vector4 clip = m_projMatStack.top() * m_mvMatrixStack.top() * Vector4(v.x, v.y, v.z, 1.0f)
```
Store clip-space results in `m_clipBuffer` (Vector4 array).

### Stage 2: PerspectiveDivide
For each clip-space vertex:
```
ndc.x = clip.x / clip.w
ndc.y = clip.y / clip.w
ndc.z = clip.z / clip.w
```
Store in `m_ndcBuffer` (Vector3 array).

### Stage 3: ViewportTransform
Map NDC `[-1, 1]` to screen pixel space using the viewport rect `(x, y, width, height)`:
```
screen.x = (ndc.x + 1.0f) * 0.5f * viewport.width  + viewport.x
screen.y = (1.0f - ndc.y) * 0.5f * viewport.height + viewport.y   // Y flipped
screen.z = ndc.z  // carry z for depth test
```
Store results in `m_screenBuffer` (Vector3 array, z carries NDC depth).

### Stage 4: RasterizeTriangle
For each assembled triangle:
- Iterate pixels in bounding box of triangle screen coordinates
- Compute edge functions for each sample point `(j + 0.5f, i + 0.5f)` where `j` is x (column) and `i` is y (row)
- If all edge functions >= 0, pixel is inside triangle
- Compute barycentric weights `(w0, w1, w2)` normalized by triangle area
- Interpolate color: `color = w0*c0 + w1*c1 + w2*c2`
- Interpolate depth: `z = w0*z0 + w1*z1 + w2*z2`
- Pass to fragment tests

### Stage 5: RunFragmentTests
For each covered pixel `(px, py)`:
```
index = py * width + px
if z < m_depthBuffer[index]:
    m_framebuffer[index] = interpolated_color
    m_depthBuffer[index] = z
```
Depth buffer initialized to `+infinity` (or `1.0f` if using normalized depth) at `glClear(GL_DEPTH_BUFFER_BIT)`.

### Stage 6: WriteOutput
Convert `m_framebuffer` (float RGB) to `uint8` and call `stbi_write_png("output.png", ...)`.

---

## Bug Fixes

### Rotation Matrix (Common.h ~line 321)
**Current (broken):**
```cpp
rotation.row[0] = { cos + axisT[0]*axis[0], axisT[0]*axis[2] - sin*axis[1], axisT[0]*axis[2] - sin*axis[1], 0.0f };
```
Columns 1 and 2 of row[0] are identical.

**Correct (Rodrigues formula):**
```cpp
rotation.row[0] = { cos + t*x*x,   t*x*y - s*z,  t*x*z + s*y,  0.0f };
rotation.row[1] = { t*x*y + s*z,   cos + t*y*y,  t*y*z - s*x,  0.0f };
rotation.row[2] = { t*x*z - s*y,   t*y*z + s*x,  cos + t*z*z,  0.0f };
// where t = 1 - cos(angle), s = sin(angle), x/y/z = normalized axis
```

### glColor3f / glVertex* Interaction (Core.cpp)
**Current:** `glColor3f` writes directly to `m_colorBuffer` (timing unclear).  
**Fix:** `glColor3f` writes to `m_currentColor` (a single `Vector3` latch on `Context`). Each `glVertex*` call snapshots `m_currentColor` into `m_colorBuffer` alongside the vertex position. This matches GL 1.1 spec: color is current state captured at vertex time.

### Rasterizer Sample Coordinates (Context.cpp ~line 50)
**Current:** `Vector2 sample = { i * 0.5f, j * 0.5f }` — arbitrary scaling, wrong axis order.  
**Fix:** `Vector2 sample = { (float)j + 0.5f, (float)i + 0.5f }` where `j` iterates columns (x) and `i` iterates rows (y). Loop bounds: `j` in `[0, width)`, `i` in `[0, height)`.

### Framebuffer Separation (Context.h)
**Current:** `m_colorBuffer` is used both for per-vertex colors during draw AND as pixel output during rasterization — dual purpose causes confusion.  
**Fix:** Add `m_framebuffer` (a `std::vector<Vector3>` of size `width * height`) as the pixel output array. `m_colorBuffer` remains per-vertex colors only.

---

## Data Structures Added to Context

```cpp
Vector3          m_currentColor;                // GL color latch (set by glColor3f)
std::vector<Vector4> m_clipBuffer;              // clip-space vertices post-MVP
std::vector<Vector3> m_ndcBuffer;              // NDC vertices post-perspective-divide  
std::vector<Vector3> m_screenBuffer;           // screen-space vertices post-viewport
std::vector<Vector3> m_framebuffer;            // pixel output (width * height)
```

`m_depthBuffer` already exists — just needs to be initialized to `FLT_MAX` on `glClear(GL_DEPTH_BUFFER_BIT)` and tested in `RunFragmentTests()`.

---

## Success Criteria

The design is complete when `OpenGLTestApp/main.cpp` (the rotating pyramid) produces a PNG file showing:
1. A correctly shaped pyramid (not a flat/degenerate quad)
2. Per-face colors correctly interpolated (not a single flat color)
3. Back faces hidden by depth testing (closer faces occlude farther ones)
4. No obvious corruption (no out-of-bounds artifacts, no all-black or all-white output)

---

## What This Does NOT Change

- No windowing system; PNG output is sufficient
- No texture, lighting, or fog support
- No additional primitive types beyond `GL_TRIANGLES`
- No changes to `gl.h` function signatures
- `glFinish()` and `glFlush()` remain equivalent (both trigger full rasterization + PNG write)
