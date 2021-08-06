#include "Board.h"
#include "Application.h"
#include "Engine.h"
#include "HSLColor.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "Tile.h"
#define NOMINMAX

namespace sam
{

    Tile::Tile(const Loc& l) : m_image(-1), m_l(l), m_needRecalc(true),
        m_buildFrame(0),
        m_dataready(false),
        m_terrain()
    {
        NoiseGen();
    }

    void Tile::SetNeighbor(int dx, int dy, std::weak_ptr<Tile> sq)
    {
        m_neighbors[(dy + 1) * 3 + (dx + 1)] = sq;
        if ((dx == -1 && dy == 0) ||
            (dy == -1 && dx == 0))
            m_needRecalc = true;
    }


    inline float cHiehgt(float n1, float n2) { return std::max(0.0f, (n2 + n1 * 1.5f) / 2.5f - 0.2f); }
    const SimplexNoise simplex;

    void Tile::NoiseGen()
    {
        float avgn1 = 0;
        float avgn2 = 0;
        int wx = m_l.m_x;
        int wy = m_l.m_z;
        float nx = wx * SquarePtsCt;
        float ny = wy * SquarePtsCt;
        float scale = 1 / (float)SquarePtsCt;

        for (int oy = 0; oy < TotalPtsCt; ++oy)
        {
            for (int ox = 0; ox < TotalPtsCt; ++ox)
            {
                int orx = ox - OverlapPtsCt;
                int ory = oy - OverlapPtsCt;
                float n1 = simplex.fractal(10, (float)(nx + orx) * scale, (float)(ny + ory) * scale) * 0.25f + 0.5f;
                float n2 = simplex.fractal(5, (float)(nx + orx) * scale * 0.5f, (float)(ny + ory) * scale * 0.5f) * 0.25f + 0.5f;
                avgn1 += n1;
                avgn2 += n2;
                m_pts[oy * TotalPtsCt + ox] = cHiehgt(n1, n2);
            }
        }
        avgn1 /= (float)(SquarePtsCt * SquarePtsCt);
        avgn2 /= (float)(SquarePtsCt * SquarePtsCt);
        SetVals(Vec2f(avgn1, avgn2));
    }

    float Tile::GetGroundHeight(const Point3f& pt) const
    {
        if (!m_dataready)
            return 0;
        int wx = m_l.m_x;
        int wz = m_l.m_z;
        float nx = wx * SquarePtsCt;
        float nz = wz * SquarePtsCt;
        float tileU = pt[0] - wx;
        float tileV = pt[2] - wz;
        tileU *= SquarePtsCt;
        tileV *= SquarePtsCt;

        return m_heightData[(int)tileV * SquarePtsCt + (int)tileU];
    }

    inline float RandF()
    {
        static const float RM = 1.0f / RAND_MAX;
        return rand() * RM;
    }

    inline float Lerp(float l, float r, float t)
    {
        return l * (1 - t) + r * t;
    }

    void Tile::ProceduralBuild(DrawContext& ctx)
    {
        int wx = m_l.m_x;
        int wy = m_l.m_z;
        float nx = wx * SquarePtsCt;
        float ny = wy * SquarePtsCt;

        const bgfx::Memory* m = bgfx::alloc(TotalPtsCt * TotalPtsCt * sizeof(Vec4f));
        Vec4f* flData = (Vec4f*)m->data;

        for (int oy = 0; oy < TotalPtsCt; ++oy)
        {
            for (int ox = 0; ox < TotalPtsCt; ++ox)
            {
                float val = m_pts[oy * TotalPtsCt + ox];
                flData[oy * TotalPtsCt + ox] = Vec4f(0, 0, 0, val);
            }
        }

        for (int i = 0; i < 2; ++i)
        {
            m_tex[i] = bgfx::createTexture2D(
                TotalPtsCt, TotalPtsCt, false,
                1,
                bgfx::TextureFormat::Enum::RGBA32F,
                BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_NONE,
                i == 0 ? m : nullptr
            );
        }

        m_terrain = bgfx::createTexture2D(
            SquarePtsCt, SquarePtsCt, false,
            1,
            bgfx::TextureFormat::Enum::R32F,
            BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT
            | BGFX_SAMPLER_MIP_POINT
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP,
            nullptr
        );
    }

    inline void AABoxAdd(AABoxf& aab, const Point3f& pt)
    {
        if (aab.isEmpty())
        {
            aab.setEmpty(false);
            aab.setMax(pt);
            aab.setMin(pt);
        }
        else
        {
            const Point3f& min = aab.getMin();
            aab.setMin(Point3f(pt[0] < min[0] ? pt[0] : min[0],
                pt[1] < min[1] ? pt[1] : min[1],
                pt[2] < min[2] ? pt[2] : min[2]));

            const Point3f& max = aab.getMax();
            aab.setMax(Point3f(pt[0] > max[0] ? pt[0] : max[0],
                pt[1] > max[1] ? pt[1] : max[1],
                pt[2] > max[2] ? pt[2] : max[2]));
        }
    }

    AABoxf Tile::GetBounds() const
    {
        const int padding = 2;
        Matrix44f m = CalcMat() *
            makeScale<Matrix44f>(Vec3f(
                (float)(m_squareSize / 2 - padding), (float)(m_squareSize / 2 - padding), 0));

        Point3f pts[4] = { Point3f(-1, -1, 0),
            Point3f(1, -1, 0) ,
            Point3f(1, 1, 0) ,
                Point3f(-1, -1, 0) };

        AABoxf aab;
        for (int idx = 0; idx < 4; ++idx)
        {
            Point3f p1;
            xform(p1, m, Point3f(-1, -1, 0));
            AABoxAdd(aab, p1);
        }
        return aab;
    }

    void Tile::Draw(DrawContext& ctx)
    {
        if (m_needRecalc)
        {
            if (m_heightData.size() == 0)
            {
                ProceduralBuild(ctx);
                m_buildFrame = 0;
            }
            else
            {
                const bgfx::Memory* m = bgfx::alloc(SquarePtsCt * SquarePtsCt * sizeof(float));
                memcpy(m->data, m_heightData.data(), m_heightData.size() * sizeof(float));

                m_terrain = bgfx::createTexture2D(
                    SquarePtsCt, SquarePtsCt, false,
                    1,
                    bgfx::TextureFormat::Enum::R32F,
                    BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_POINT
                    | BGFX_SAMPLER_MAG_POINT
                    | BGFX_SAMPLER_MIP_POINT
                    | BGFX_SAMPLER_U_CLAMP
                    | BGFX_SAMPLER_V_CLAMP,
                    m
                );
            }
            m_needRecalc = false;
        }
        m_texpingpong = 0;
        if (m_buildFrame == 0)
        {
            for (int i = 0; i < 5000; i++)
            {
                bgfx::setTexture(0, ctx.m_texture, m_tex[m_texpingpong]);
                bgfx::setImage(1, m_tex[1 - m_texpingpong], 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
                bgfx::dispatch(0, Engine::Inst().m_erosion, TotalPtsCt / 16, TotalPtsCt / 16);
                m_texpingpong = 1 - m_texpingpong;
            }

            bgfx::setTexture(0, ctx.m_texture, m_tex[m_texpingpong]);
            bgfx::setImage(1, m_terrain, 0, bgfx::Access::Write, bgfx::TextureFormat::R32F);
            bgfx::dispatch(0, Engine::Inst().m_copysect, SquarePtsCt / 16, SquarePtsCt / 16);
            m_buildFrame++;
        }
        else if (m_buildFrame == 1)
        {
            m_rbTex = bgfx::createTexture2D(
                SquarePtsCt, SquarePtsCt, false,
                1,
                bgfx::TextureFormat::Enum::R32F,
                BGFX_TEXTURE_BLIT_DST
                | BGFX_TEXTURE_READ_BACK
                | BGFX_SAMPLER_MIN_POINT
                | BGFX_SAMPLER_MAG_POINT
                | BGFX_SAMPLER_MIP_POINT
                | BGFX_SAMPLER_U_CLAMP
                | BGFX_SAMPLER_V_CLAMP);
            bgfx::blit(0, m_rbTex, 0, 0, m_terrain);
            m_heightData.resize(SquarePtsCt * SquarePtsCt);
            m_buildFrame = bgfx::readTexture(m_rbTex, m_heightData.data());
        }
        else if (ctx.m_frameIdx == m_buildFrame)
        {
            bgfx::destroy(m_rbTex);
            m_dataready = true;
            m_buildFrame = -1;
        }
        float y = 2;// std::min(0.0f, -val * 4);

        Matrix44f m =
            ctx.m_mat * CalcMat() *
            makeScale<Matrix44f>(Vec3f(
                (float)(m_squareSize / 2), (float)(m_squareSize), (float)(m_squareSize / 2))) *
            makeTrans<Matrix44f>(Vec3f(0, 0, 0));


        bgfx::setTransform(m.getData());
        bgfx::setTexture(0, ctx.m_texture, m_terrain);

        Grid::init();

        // Set vertex and index buffer.
        bgfx::setVertexBuffer(0, Grid::vbh);
        bgfx::setIndexBuffer(Grid::ibh);
        uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CCW
            | BGFX_STATE_MSAA;
        // Set render states.l
        bgfx::setState(state);
        bgfx::submit(0, ctx.m_pgm);
    }

    void Tile::Decomission()
    {
        for (int i = 0; i < 2; ++i)
        {
            if (bgfx::isValid(m_tex[i]))
            {
                bgfx::destroy(m_tex[i]);
                m_tex[i] = BGFX_INVALID_HANDLE;
            }
        }
        if (bgfx::isValid(m_terrain))
        {
            bgfx::destroy(m_terrain);
            m_terrain = BGFX_INVALID_HANDLE;
        }
        m_needRecalc = true;
    }

    Tile::~Tile()
    {
        if (m_image >= 0)
        {
        }
    }
}