#include "SceneItem.h"
#include <bgfx/bgfx.h>
#include <gmtl/FrustumOps.h>
#include "Mesh.h"

using namespace gmtl;

SceneItem::SceneItem() :
    m_offset(0, 0, 0),
    m_scale(1, 1, 1),
    m_rotate()
{
}

Matrix44f SceneItem::CalcMat() const
{
    return makeTrans<Matrix44f>(m_offset) *
        makeRot<Matrix44f>(m_rotate) *
        makeScale<Matrix44f>(m_scale);
}

inline void AABoxAdd(AABoxf& aab1, const AABoxf& aab2)
{
    if (aab1.isEmpty())
    {
        aab1 = aab2;
    }
    else
    {
        {
            const Point3f& min = aab1.getMin();
            const Point3f& pt = aab2.getMin();
            aab1.setMin(Point3f(pt[0] < min[0] ? pt[0] : min[0],
                pt[1] < min[1] ? pt[1] : min[1],
                pt[2] < min[2] ? pt[2] : min[2]));
        }

        {
            const Point3f& pt = aab2.getMax();
            const Point3f& max = aab1.getMax();
            aab1.setMax(Point3f(pt[0] > max[0] ? pt[0] : max[0],
                pt[1] > max[1] ? pt[1] : max[1],
                pt[2] > max[2] ? pt[2] : max[2]));
        }
    }
}

AABoxf SceneGroup::GetBounds() const
{
    AABoxf bnds;
    for (auto item : m_sceneItems)
    {
        AABoxAdd(bnds, item->GetBounds());
    }

    Point3f pt;
    Matrix44f m = CalcMat();
    xform(pt, m, bnds.getMin());
    bnds.setMin(pt);
    xform(pt, m, bnds.getMax());
    bnds.setMax(pt);
    return bnds;
}

void SceneGroup::Draw(DrawContext& ctx)
{
    Matrix44f prevMat = ctx.m_mat;
    for (auto item : m_sceneItems)
    {
        ctx.m_mat *= CalcMat();
        item->Draw(ctx);
        ctx.m_mat = prevMat;
    }
}

void SceneText::SetText(const std::string& text)
{
    m_text = text;
}

void SceneText::SetFont(const std::string& fontname, float size)
{
    m_font = fontname;
    m_size = size;
}

void SceneText::Draw(DrawContext& ctx)
{
    Matrix44f m =
        ctx.m_mat * CalcMat();

    Point3f pt = makeTrans<Point3f>(m);
}

AABoxf SceneText::GetBounds() const
{
    Matrix44f m = CalcMat();
    return AABoxf();
    //nvgTextBounds()
}

void SceneRect_DrawCube()
{
    Cube::init();

    // Set vertex and index buffer.
    bgfx::setVertexBuffer(0, Cube::vbh);
    bgfx::setIndexBuffer(Cube::ibh);
    uint64_t state = 0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA;
    // Set render states.
    bgfx::setState(state);
}

void SceneRect::Draw(DrawContext& ctx)
{
    const int padding = 2;
    Matrix44f m =
        ctx.m_mat * CalcMat();
    
    SceneRect_DrawCube();

    Cube::init();
    bgfx::submit(0, ctx.m_pgm);
}

AABoxf SceneRect::GetBounds() const
{
    Matrix44f m = CalcMat();
    return AABoxf();
    //nvgTextBounds()
}

static const int boardSizeW = 32;
static const int boardSizeH = 48;

Camera::Camera() 
{}

void Camera::Update(int w, int h)
{
    Matrix44f rot, off, scl, perp;
   
    float aspect = (float)w / (float)h;
    setPerspective(m_proj, 60.0f, aspect, 0.1f, 10.0f);    
    setRot(rot, AxisAnglef(Math::PI + m_lookat.tilt, 1.0f, 0.0f, 0.0f));
    Vec3f vec(0, 0, m_lookat.dist);
    Quatf q = make<gmtl::Quatf>(AxisAnglef(Math::PI + m_lookat.tilt, 1.0f, 0.0f, 0.0f));
    vec = q * vec;    
    setTrans(off, m_lookat.pos + vec);
    m_view = off * rot;
    m_view = invert(m_view);    
}

Frustumf Camera::GetFrustum() const
{
    Frustumf f(m_view, m_proj);
    normalize(f);
    return f;
}