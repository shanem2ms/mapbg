#include "StdIncludes.h"
#include "World.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "TerrainTile.h"
#define NOMINMAX

namespace sam
{

    TerrainTile::TerrainTile(const Loc& l, std::shared_ptr<TerrainTile> parent) :
        m_image(-1), m_l(l),
        m_needRecalc(true),
        m_buildFrame(0),
        m_buildStep(0),
        m_dataready(false),
        m_terrain(BGFX_INVALID_HANDLE),
        m_rbTex(BGFX_INVALID_HANDLE),
        m_parent(parent),
        m_lastUsedframeIdx(0)
    {
        m_tex[0] = m_tex[1] = BGFX_INVALID_HANDLE;
    }


    inline float cHiehgt(float n1, float n2) { return (n2 + n1 * 1.5f) / 2.5f - 0.5f; }
    const SimplexNoise simplex;

    void TerrainTile::NoiseGen()
    {
        const float maxTerrainHeight = 2.0f;
        float avgn1 = 0;
        float avgn2 = 0;
        AABoxf box = m_l.GetBBox();

        float extents = m_l.GetExtent();

        float terrainHeight = maxTerrainHeight / (1 << m_l.m_l);

        float scale = extents / SquarePtsCt;
        constexpr float ovfl = (float)OverlapPtsCt / (float)(SquarePtsCt);
        float nx = box.mMin[0] - extents * ovfl;
        float ny = box.mMin[2] - extents * ovfl;

        int pX = 0, pZ = 0;
        if (m_parent != nullptr)
        {
            Loc ll = m_l.GetLocal(m_parent->m_l);
            pX = ll.m_x;
            pZ = ll.m_z;
        }
        float minheight = std::numeric_limits<float>::max();
        float maxheight = -minheight;
        m_pts.resize(TotalPtsCt * TotalPtsCt);
        for (int oy = 0; oy < TotalPtsCt; ++oy)
        {
            for (int ox = 0; ox < TotalPtsCt; ++ox)
            {
                float n1 = simplex.fractal(10, nx + ox * scale, ny + oy * scale) * 0.25f + 0.5f;
                float n2 = simplex.fractal(5, (nx + ox * scale) * 0.5f, (ny + oy * scale) * 0.5f) * 0.25f + 0.5f;
                avgn1 += n1;
                avgn2 += n2;
                float h = cHiehgt(n1, n2);
                if (ox > OverlapPtsCt && ox < (SquarePtsCt + OverlapPtsCt) &&
                    oy > OverlapPtsCt && oy < (SquarePtsCt + OverlapPtsCt))
                {
                    minheight = std::min(minheight, h);
                    maxheight = std::max(maxheight, h);
                }
                m_pts[oy * TotalPtsCt + ox] = h;
            }
        }
        avgn1 /= (float)(SquarePtsCt * SquarePtsCt);
        avgn2 /= (float)(SquarePtsCt * SquarePtsCt);
        SetVals(Vec2f(avgn1, avgn2));
    }

    float TerrainTile::GetGroundHeight(const Point3f& pt) const
    {
        if (!m_dataready)
            return 0;
        int wx = m_l.m_x;
        int wz = m_l.m_z;
        float nx = wx * SquarePtsCt;
        float nz = wz * SquarePtsCt;
        float tileU = pt[0] - wx;
        float tileV = pt[2] - wz;
        tileU *= (SquarePtsCt - 1);
        tileV *= (SquarePtsCt - 1);

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

    bool TerrainTile::Build()
    {
        //if (m_buildStep == 0)
            //NoiseGen();
        return GpuErosion();
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

    bgfx::UniformHandle TerrainTile::m_texture = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle TerrainTile::m_erosion = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle TerrainTile::m_copysect = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle TerrainTile::m_csnoise = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle TerrainTile::m_uparams = BGFX_INVALID_HANDLE;

    bool TerrainTile::GpuErosion()
    {
        int frameIdx = Application::Inst().FrameIdx();
        m_texpingpong = 0;
        if (m_buildStep == 0)
        {
            if (!bgfx::isValid(m_texture)) {
                m_texture = bgfx::createUniform("s_terrain", bgfx::UniformType::Sampler);
                m_erosion = Engine::Inst().LoadShader("cs_erosion.bin");
                m_copysect = Engine::Inst().LoadShader("cs_copysect.bin");
                m_csnoise = Engine::Inst().LoadShader("cs_noise.bin");
                m_uparams = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, 1);
            }

            int wx = m_l.m_x;
            int wy = m_l.m_z;
            float nx = wx * SquarePtsCt;
            float ny = wy * SquarePtsCt;
            /*
            const bgfx::Memory* m = bgfx::alloc(TotalPtsCt * TotalPtsCt * sizeof(Vec4f));
            Vec4f* flData = (Vec4f*)m->data;

            for (int oy = 0; oy < TotalPtsCt; ++oy)
            {
                for (int ox = 0; ox < TotalPtsCt; ++ox)
                {
                    float val = m_pts[oy * TotalPtsCt + ox];
                    flData[oy * TotalPtsCt + ox] = Vec4f(0, 0, 0, val);
                }
            }*/

            for (int i = 0; i < 2; ++i)
            {
                m_tex[i] = bgfx::createTexture2D(
                    TotalPtsCt, TotalPtsCt, false,
                    1,
                    bgfx::TextureFormat::Enum::RGBA32F,
                    BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_NONE,
                    nullptr
                );
            }

            m_terrain = bgfx::createTexture2D(
                TotalPtsCt, TotalPtsCt, false,
                1,
                bgfx::TextureFormat::Enum::R32F,
                BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_POINT
                | BGFX_SAMPLER_MAG_POINT
                | BGFX_SAMPLER_MIP_POINT
                | BGFX_SAMPLER_U_CLAMP
                | BGFX_SAMPLER_V_CLAMP,
                nullptr
            );

            AABoxf box = m_l.GetBBox();
            float extents = m_l.GetExtent();

            float overlap = (float)OverlapPtsCt / (float)SquarePtsCt;
            overlap *= extents;
            Vec4f tileScaleOffset(
                (extents + overlap * 2), 
                (extents + overlap * 2),
                (box.mMin[0] - overlap), 
                (box.mMin[2] - overlap));
            bgfx::setUniform(m_uparams, &tileScaleOffset, 1);
            bgfx::setImage(0, m_tex[0], 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
            bgfx::dispatch(0, m_csnoise, TotalPtsCt / 16, TotalPtsCt / 16);
            /*
            for (int i = 0; i < 1000; i++)
            {
                bgfx::setTexture(0, m_texture, m_tex[m_texpingpong]);
                bgfx::setImage(1, m_tex[1 - m_texpingpong], 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
                bgfx::dispatch(0, m_erosion, TotalPtsCt / 16, TotalPtsCt / 16);
                m_texpingpong = 1 - m_texpingpong;
            }
            */
            bgfx::setTexture(0, m_texture, m_tex[m_texpingpong]);
            bgfx::setImage(1, m_terrain, 0, bgfx::Access::Write, bgfx::TextureFormat::R32F);
            bgfx::dispatch(0, m_copysect, TotalPtsCt / 16, TotalPtsCt / 16);
            m_buildStep = 1;
            m_buildFrame = frameIdx + 1;
        }
        else if (m_buildStep == 1 && frameIdx >= m_buildFrame)
        {
            m_rbTex = bgfx::createTexture2D(
                TotalPtsCt, TotalPtsCt, false,
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
            m_heightData.resize(TotalPtsCt * TotalPtsCt);
            m_buildFrame = bgfx::readTexture(m_rbTex, m_heightData.data());
            m_buildStep = 2;
        }
        else if (m_buildStep == 2 && frameIdx >= m_buildFrame)
        {
            float minheight = std::numeric_limits<float>::max();
            float maxheight = -minheight;
            for (float val : m_heightData)
            {
                minheight = std::min(minheight, val);
                maxheight = std::max(maxheight, val);
            }
            m_heightBbox = m_l.GetBBox();
            m_heightBbox.mMin[1] = minheight;
            m_heightBbox.mMax[1] = maxheight;

            bgfx::destroy(m_rbTex);
            m_rbTex = BGFX_INVALID_HANDLE;
            m_dataready = true;
            m_buildStep = -1;
        }

        return m_dataready;
    }


    void TerrainTile::Decomission()
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

        if (bgfx::isValid(m_uparams))
        {
            bgfx::destroy(m_uparams);
            m_uparams = BGFX_INVALID_HANDLE;
        }
        if (bgfx::isValid(m_rbTex))
        {
            bgfx::destroy(m_rbTex);
            m_rbTex = BGFX_INVALID_HANDLE;
        }
        m_needRecalc = true;
    }

    TerrainTile::~TerrainTile()
    {
        if (m_image >= 0)
        {
        }
    }
}