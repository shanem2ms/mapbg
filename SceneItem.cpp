#include "SceneItem.h"
#include <bgfx/bgfx.h>

using namespace gmtl;

struct PosTexcoordVertex
{
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    };

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosTexcoordVertex::ms_layout;

struct Cube
{
    static void init()
    {
        if (isInit)
            return;
        PosTexcoordVertex::init();


        static PosTexcoordVertex s_cubeVertices[] =
        {
            {-1.0f,  1.0f,  1.0f,  0.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f,  1.0f,  1.0f},
            {-1.0f, -1.0f,  1.0f,  0.0f,  0.0f},
            { 1.0f, -1.0f,  1.0f,  1.0f,  0.0f},

            {-1.0f,  1.0f, -1.0f,  0.0f,  1.0f},
            { 1.0f,  1.0f, -1.0f,  1.0f,  1.0f},
            {-1.0f, -1.0f, -1.0f,  0.0f,  0.0f},
            { 1.0f, -1.0f, -1.0f,  1.0f,  0.0f},

            {-1.0f,  1.0f,  1.0f, 0.0f,  1.0f},
            {-1.0f,  1.0f, -1.0f, 1.0f,  1.0f},
            {-1.0f, -1.0f,  1.0f, 0.0f,  0.0f},
            {-1.0f, -1.0f, -1.0f, 1.0f,  0.0f},

            { 1.0f,  1.0f,  1.0f, 0.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f, 1.0f,  1.0f},
            { 1.0f,  1.0f, -1.0f, 0.0f,  0.0f},
            { 1.0f, -1.0f, -1.0f, 1.0f,  0.0f},

            {-1.0f,  1.0f,  1.0f, 0.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f, 1.0f,  1.0f},
            {-1.0f,  1.0f, -1.0f, 0.0f,  0.0f},
            { 1.0f,  1.0f, -1.0f, 1.0f,  0.0f},

            {-1.0f, -1.0f,  1.0f, 0.0f,  1.0f},
            {-1.0f, -1.0f, -1.0f, 1.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f, 0.0f,  0.0f},
            { 1.0f, -1.0f, -1.0f, 1.0f,  0.0f},
        };

        static const uint16_t s_cubeIndices[] =
        {
             0,  1,  2, // 0
             1,  3,  2,

             4,  6,  5, // 2
             5,  6,  7,

             8, 10,  9, // 4
             9, 10, 11,

            12, 14, 13, // 6
            14, 15, 13,

            16, 18, 17, // 8
            18, 19, 17,

            20, 22, 21, // 10
            21, 22, 23,
        };

        vbh = bgfx::createVertexBuffer(
            bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices))
            , PosTexcoordVertex::ms_layout
        );

        ibh = bgfx::createIndexBuffer(
            bgfx::makeRef(s_cubeIndices, sizeof(s_cubeIndices))
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

gmtl::AABoxf SceneRect::GetBounds() const
{
    gmtl::Matrix44f m = CalcMat();
    return gmtl::AABoxf();
    //nvgTextBounds()
}

static const int boardSizeW = 32;
static const int boardSizeH = 48;

Camera::Camera() : m_pos(0, 0, -1.0f) {}

void Camera::Update(int w, int h)
{
    gmtl::Matrix44f rot, off, scl, perp;
   
    float aspect = (float)w / (float)h;
    gmtl::setPerspective(m_persp, 60.0f, aspect, 0.1f, 10.0f);

    gmtl::setRot(rot, gmtl::AxisAnglef(gmtl::Math::PI, 0.0f, 1.0f, 0.0f));
    gmtl::setTrans(off, m_pos);

    m_view = off * rot;
    m_view = invert(m_view);
}