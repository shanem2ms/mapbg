#include "World.h"
#include "Application.h"
#include "Engine.h"
#include "HSLColor.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "OctTile.h"
#define NOMINMAX

namespace sam
{

    OctTile::OctTile(const Loc& l) : m_image(-1), m_l(l), m_needRecalc(true),
        m_buildFrame(0),
        m_dataready(false),
        m_uparams(BGFX_INVALID_HANDLE)
    {
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

    AABoxf OctTile::GetBounds() const
    {
        const int padding = 2;
        Matrix44f m = CalcMat() *
            makeScale<Matrix44f>(Vec3f(
                (float)(1.0f / 2 - padding), (float)(1.0f / 2 - padding), 0));

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


    void OctTile::Draw(DrawContext& ctx)
    {
        if (!bgfx::isValid(m_uparams))
        {
            m_uparams = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, 1);
        }

        int t = m_l.m_l;

        Vec4f color((std::min(t, 93) + 1) * 0.1f,
            (std::max(t - 10, 0) + 1) * 0.1f,
            1,
            1);
        bgfx::setUniform(m_uparams, &color, 1);
        Matrix44f m =
            ctx.m_mat * CalcMat();


        bgfx::setTransform(m.getData());
        Grid<16>::init();

        // Set vertex and index buffer.
        bgfx::setVertexBuffer(0, Grid<16>::vbh);
        bgfx::setIndexBuffer(Grid<16>::ibh);
        uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA;
        // Set render states.l
        bgfx::setState(state);
        bgfx::submit(0, ctx.m_pgm);
    }

    void OctTile::Decomission()
    {

        if (bgfx::isValid(m_uparams))
        {
            bgfx::destroy(m_uparams);
            m_uparams = BGFX_INVALID_HANDLE;
        }
        m_needRecalc = true;
    }

    OctTile::~OctTile()
    {
        if (m_image >= 0)
        {
        }
    }
}