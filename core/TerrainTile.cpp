#include "StdIncludes.h"
#include "World.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "TerrainTile.h"
#include "leveldb/db.h"
#define NOMINMAX

namespace sam
{

    const int numErosionIters = 1000;

    TerrainTile::TerrainTile(const Loc& l, std::shared_ptr<TerrainTile> parent) :
        m_image(-1), m_l(l),
        m_needRecalc(true),
        m_buildFrame(0),
        m_buildStep(0),
        m_dataready(false),
        m_parent(parent),
        m_lastUsedframeIdx(0)
    {
    }


    inline float cHiehgt(float n1, float n2) { return (n2 + n1 * 1.5f) / 2.5f - 0.5f; }
    const SimplexNoise simplex;

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
    bgfx::ProgramHandle TerrainTile::m_cscopyparent = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle TerrainTile::m_uparams = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle TerrainTile::m_vparams = BGFX_INVALID_HANDLE;

    static std::set<Loc> sAllLocs;

    bool TerrainTile::Build(World* pWorld)
    {
        int frameIdx = Application::Inst().FrameIdx();
        m_texpingpong = 0;
        auto parent = m_parent;
        if (parent != nullptr && !parent->IsDataReady())
            return false;

        if (m_buildStep == 0)
        {
            Loc l = m_l;
            l.m_l = -m_l.m_l;
            leveldb::Slice key((const char*)&l, sizeof(l));
            leveldb::DB* db = pWorld->Db();
            std::string val;
            leveldb::Status status = db->Get(leveldb::ReadOptions(), key, &val);

            if (status.ok())
            {
                m_heightData.resize(val.size() / sizeof(float));
                memcpy(m_heightData.data(), val.data(), m_heightData.size() * sizeof(float));

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

                m_parent = nullptr;

                m_dataready = true;
                m_buildStep = -1;
            }
            else if (status.IsNotFound())
            {
                sAllLocs.insert(m_l);
                if (!bgfx::isValid(m_texture)) {
                    m_texture = bgfx::createUniform("s_terrain", bgfx::UniformType::Sampler);
                    m_erosion = Engine::Inst().LoadShader("cs_erosion.bin");
                    m_copysect = Engine::Inst().LoadShader("cs_copysect.bin");
                    m_csnoise = Engine::Inst().LoadShader("cs_noise.bin");
                    m_cscopyparent = Engine::Inst().LoadShader("cs_copyparent.bin");
                    m_uparams = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, 1);
                    m_vparams = bgfx::createUniform("v_params", bgfx::UniformType::Vec4, 1);
                }

                m_terrain = bgfx::createTexture2D(
                    TotalPtsCt, TotalPtsCt, false,
                    1,
                    bgfx::TextureFormat::Enum::R32F,
                    BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_NONE
                    | BGFX_SAMPLER_U_CLAMP
                    | BGFX_SAMPLER_V_CLAMP,
                    nullptr
                );

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

                AABoxf box = m_l.GetBBox();
                float extents = m_l.GetExtent();

                float overlap = (float)OverlapPtsCt / (float)SquarePtsCt;
                overlap *= extents;

                if (parent == nullptr || m_l.m_l < 5)
                {
                    Vec4f tileScaleOffset(
                        (extents + overlap * 2),
                        1,
                        (box.mMin[0] - overlap),
                        (box.mMin[2] - overlap));

                    int wx = m_l.m_x;
                    int wy = m_l.m_z;
                    float nx = wx * SquarePtsCt;
                    float ny = wy * SquarePtsCt;

                    bgfx::setUniform(m_uparams, &tileScaleOffset, 1);
                    bgfx::setImage(0, m_tex[0], 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
                    bgfx::dispatch(0, m_csnoise, TotalPtsCt / 16, TotalPtsCt / 16);

                    for (int i = 0; i < numErosionIters; i++)
                    {
                        bgfx::setTexture(0, m_texture, m_tex[m_texpingpong]);
                        bgfx::setImage(1, m_tex[1 - m_texpingpong], 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
                        bgfx::dispatch(0, m_erosion, TotalPtsCt / 16, TotalPtsCt / 16);
                        m_texpingpong = 1 - m_texpingpong;
                    }

                    bgfx::setTexture(0, m_texture, m_tex[m_texpingpong]);
                    bgfx::setImage(1, m_terrain, 0, bgfx::Access::Write, bgfx::TextureFormat::R32F);
                    bgfx::dispatch(0, m_copysect, TotalPtsCt / 16, TotalPtsCt / 16);
                    m_buildStep = 1;
                    m_buildFrame = frameIdx + 1;
                }
                else
                {

                    Vec4f tileScaleOffset(
                        1.0f,
                        extents / 16.0f,
                        (float)m_l.m_x,
                        (float)m_l.m_z);

                    constexpr float xParentOff[2] = {
                        (float)(OverlapPtsCt - OverlapPtsCt / 2),
                        (float)(OverlapPtsCt + (SquarePtsCt / 2) - OverlapPtsCt / 2) };
                    constexpr float xChildOff = (float)(OverlapPtsCt / 2) / (float)TotalPtsCt;

                    Loc ll = m_l.GetLocal(m_l.Parent());
                    Vec4f parentOffset(
                        0.5,
                        0.5,
                        xParentOff[ll.m_x],
                        xParentOff[ll.m_z]);
                    bgfx::setUniform(m_uparams, &tileScaleOffset, 1);
                    bgfx::setUniform(m_vparams, &parentOffset, 1);
                    Vec4f uloc(m_l.m_l, m_l.m_x, m_l.m_z, 0);
                    const auto& terrain = parent->GetTerrain();
                    bgfx::setTexture(0, m_texture, terrain);
                    bgfx::setImage(1, m_tex[0], 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
                    bgfx::dispatch(0, m_cscopyparent, TotalPtsCt / 16, TotalPtsCt / 16);

                    for (int i = 0; i < numErosionIters; i++)
                    {
                        bgfx::setTexture(0, m_texture, m_tex[m_texpingpong]);
                        bgfx::setImage(1, m_tex[1 - m_texpingpong], 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
                        bgfx::dispatch(0, m_erosion, TotalPtsCt / 16, TotalPtsCt / 16);
                        m_texpingpong = 1 - m_texpingpong;
                    }

                    bgfx::setTexture(0, m_texture, m_tex[m_texpingpong]);
                    bgfx::setImage(1, m_terrain, 0, bgfx::Access::Write, bgfx::TextureFormat::R32F);
                    bgfx::dispatch(0, m_copysect, TotalPtsCt / 16, TotalPtsCt / 16);
                    m_buildStep = 1;
                    m_buildFrame = frameIdx + 1;
                }
            }
            else 
            __debugbreak();
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

            m_parent = nullptr;
            m_rbTex.free();

            Loc l = m_l;
            l.m_l = -m_l.m_l;
            leveldb::Slice key((const char*)&l, sizeof(l));
            leveldb::Slice val((const char*)m_heightData.data(), m_heightData.size() * sizeof(float));
            leveldb::DB* db = pWorld->Db();
            leveldb::Status status = db->Put(leveldb::WriteOptions(), key, val);

            m_dataready = true;
            m_buildStep = -1;
        }

        return m_dataready;
    }



    TerrainTile::~TerrainTile()
    {
    }
}