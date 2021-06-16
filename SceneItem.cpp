#include "SceneItem.h"
#include <bgfx/bgfx.h>

using namespace gmtl;

struct PosColorVertex
{
    float m_x;
    float m_y;
    float m_z;

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .end();
    };

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosColorVertex::ms_layout;

struct Cube
{
    static void init()
    {
        if (isInit)
            return;
        PosColorVertex::init();

        static PosColorVertex s_cubeVertices[] =
        {
            {-1.0f,  1.0f,  1.0f },
            { 1.0f,  1.0f,  1.0f },
            {-1.0f, -1.0f,  1.0f },
            { 1.0f, -1.0f,  1.0f },
            {-1.0f,  1.0f, -1.0f },
            { 1.0f,  1.0f, -1.0f },
            {-1.0f, -1.0f, -1.0f },
            { 1.0f, -1.0f, -1.0f },
        };

        static const uint16_t s_cubeTriList[] =
        {
            0, 1, 2, // 0
            1, 3, 2,
            4, 6, 5, // 2
            5, 6, 7,
            0, 2, 4, // 4
            4, 2, 6,
            1, 5, 3, // 6
            5, 7, 3,
            0, 4, 1, // 8
            4, 5, 1,
            2, 3, 6, // 10
            6, 3, 7,
        };

        vbh = bgfx::createVertexBuffer(
            bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices))
            , PosColorVertex::ms_layout
        );

        ibh = bgfx::createIndexBuffer(
            bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList))
        );

        isInit = true;
    }

    static bgfx::VertexBufferHandle vbh;
    static bgfx::IndexBufferHandle ibh;
    static bool isInit;
};

bool Cube::isInit = false;
bgfx::VertexBufferHandle Cube::vbh;
bgfx::IndexBufferHandle Cube::ibh;

SceneItem::SceneItem() :
    m_offset(0, 0, 0),
    m_scale(1, 1, 1),
    m_rotate()
{
}

gmtl::Matrix44f SceneItem::CalcMat() const
{
    return gmtl::makeTrans<gmtl::Matrix44f>(m_offset) *
        gmtl::makeRot<gmtl::Matrix44f>(m_rotate) *
        gmtl::makeScale<gmtl::Matrix44f>(m_scale);
}

inline void AABoxAdd(gmtl::AABoxf& aab1, const gmtl::AABoxf& aab2)
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

gmtl::AABoxf SceneGroup::GetBounds() const
{
    gmtl::AABoxf bnds;
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
    gmtl::Matrix44f prevMat = ctx.m_mat;
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
    gmtl::Matrix44f m =
        ctx.m_mat * CalcMat();

    gmtl::Point3f pt = gmtl::makeTrans<gmtl::Point3f>(m);
}

gmtl::AABoxf SceneText::GetBounds() const
{
    gmtl::Matrix44f m = CalcMat();
    return gmtl::AABoxf();
    //nvgTextBounds()
}

void SceneRect::Draw(DrawContext& ctx)
{
    const int padding = 2;
    Matrix44f m =
        ctx.m_mat * CalcMat();

    Cube::init();

    // Set vertex and index buffer.
    bgfx::setVertexBuffer(0, Cube::vbh);
    bgfx::setIndexBuffer(Cube::ibh);
    uint64_t state = 0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CW
        | BGFX_STATE_MSAA;
    // Set render states.
    bgfx::setState(state);

    // Submit primitive for rendering to view 0.
    bgfx::submit(0, ctx.m_pgm);

}

gmtl::AABoxf SceneRect::GetBounds() const
{
    gmtl::Matrix44f m = CalcMat();
    return gmtl::AABoxf();
    //nvgTextBounds()
}



Camera::Camera() {}

void Camera::Update(int w, int h)
{
    gmtl::Matrix44f rot, off, scl, perp;
    gmtl::setScale(scl, gmtl::Vec3f(2.0f / w, -2.0f / h, 2.0f / w));
    gmtl::setTrans(off, gmtl::Vec3f(-1.0f, 1.0f, 0.5f));
    gmtl::setRot(rot, gmtl::AxisAnglef(0.0f * gmtl::Math::PI / 180.0f, 1.0f, 0.0f, 0.0f));

    float nr = 0.1f;
    float fr = 2.0f;
    perp.mState = Matrix44f::FULL;
    perp.mData[0] = 1.5f;
    perp.mData[5] = 1.5f;
    perp.mData[10] = fr / (fr - nr);;
    perp.mData[11] = 1.0f;
    perp.mData[14] = -(fr * nr) / (fr - nr);
    perp.mData[15] = 0;
    m_persp = perp * rot * off * scl;
}