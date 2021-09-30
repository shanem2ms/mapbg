#include "StdIncludes.h"
#include "World.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "OctTile.h"
#include "TerrainTileSelection.h"
#include "TerrainTile.h"
#include "gmtl/Intersection.h"
#define NOMINMAX

namespace sam
{

    bgfxh<bgfx::ProgramHandle> sBboxshader;

    OctTile::OctTile(const Loc& l) : m_image(-1), m_l(l),
        m_buildFrame(0),
        m_readyState(0),
        m_intersects(-1),
        m_lastUsedRawData(0)
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


    int nOctTilesTotal;
    int nOctTilesDrawn;

    std::vector<byte> OctTile::RleEncode(const std::vector<byte> data)
    {
        std::vector<byte> outrle;
        byte curval = data[0];
        int cnt = 0;
        for (byte v : data)
        {
            if (v != curval || cnt == 0x7FFF)
            {
                if (cnt < 0x80)
                    outrle.push_back(cnt);
                else
                {
                    byte b1 = cnt >> 8;
                    byte b2 = cnt & 0xFF;
                    b1 |= 0x80;
                    outrle.push_back(b1);
                    outrle.push_back(b2);
                }

                outrle.push_back(curval);
                cnt = 1;
                curval = v;
            }
            else
                cnt++;
        }
        if (cnt > 0)
        {
            if (cnt < 0x80)
                outrle.push_back(cnt);
            else
            {
                byte b1 = cnt >> 8;
                byte b2 = cnt & 0xFF;
                b1 |= 0x80;
                outrle.push_back(b1);
                outrle.push_back(b2);
            }
            outrle.push_back(curval);
        }
        return outrle;
    }

    std::vector<byte> OctTile::RleDecode(const std::vector<byte> data)
    {
        std::vector<byte> outdata;
        outdata.resize(TerrainTile::SquarePtsCt * TerrainTile::SquarePtsCt * TerrainTile::SquarePtsCt);
        size_t offset = 0;
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            byte cnt0 = *it;
            if (cnt0 < 0x80)
            {
                ++it;
                memset(outdata.data() + offset, *it, cnt0);
                offset += cnt0;
            }
            else
            {
                ++it;
                byte cnt1 = *it;
                cnt0 &= 0x7F;
                int size = (cnt0 << 8) | cnt1;
                ++it;
                memset(outdata.data() + offset, *it, size);
                offset += size;
            }
        }

        return outdata;
    }

    void OctTile::LoadVB()
    {
        AABoxf bboxoct = m_l.GetBBox();
        float minY = bboxoct.mMin[1];
        float maxY = bboxoct.mMax[1];
        float minX = bboxoct.mMin[0];
        float minZ = bboxoct.mMin[2];
        float len = (bboxoct.mMax[0] - bboxoct.mMin[0]) / TerrainTile::SquarePtsCt;


        const int tsz = TerrainTile::SquarePtsCt;
        std::vector<Vec3f> octPts;

        size_t offset = 0;
        for (auto it = m_rledata.begin(); it != m_rledata.end(); ++it)
        {
            byte cnt0 = *it;
            if (cnt0 < 0x80)
            {
                ++it;
                if (*it == 1)
                {
                    for (byte idx = 0; idx < cnt0; ++idx)
                    {
                        size_t v = offset + idx;
                        int y = (v / (tsz * tsz));
                        v -= y * tsz * tsz;
                        int z = v / tsz;
                        v -= z * tsz;
                        int x = v;
                        octPts.push_back(Vec3f(minX + x * len, minY + y * len, minZ + z * len));
                    }
                }
                offset += cnt0;
            }
            else
            {
                ++it;
                byte cnt1 = *it;
                cnt0 &= 0x7F;
                int size = (cnt0 << 8) | cnt1;
                ++it;
                offset += size;
            }
        }

        if (octPts.size() > 0)
        {
            m_cubeList = std::make_shared<CubeList>();
            m_cubeList->Create(octPts, len * 0.5f);
        }
    }

    void OctTile::LoadTerrainData()
    {
        AABoxf bboxterrain = m_terrainTile->GetBounds();
        AABoxf bboxoct = m_l.GetBBox();

        if (!intersect(bboxterrain, bboxoct))
            return;

        float minY = bboxoct.mMin[1];
        float maxY = bboxoct.mMax[1];
        float minX = bboxoct.mMin[0];
        float minZ = bboxoct.mMin[2];


        std::vector<byte> data;
        const int tsz = TerrainTile::SquarePtsCt;
        data.resize(tsz * tsz * tsz);

        const float* tpts = m_terrainTile->Pts();
        float len = (bboxoct.mMax[0] - bboxoct.mMin[0]) / TerrainTile::SquarePtsCt;
        float ext = TerrainTile::SquarePtsCt / (maxY - minY);

        for (int z = 0; z < TerrainTile::SquarePtsCt; ++z)
        {
            for (int x = 0; x < TerrainTile::SquarePtsCt; ++x)
            {
                int offset = (z + TerrainTile::OverlapPtsCt) * TerrainTile::TotalPtsCt + (x + TerrainTile::OverlapPtsCt);
                float h = tpts[offset];
                if (h > minY && h < maxY)
                {
                    int y = std::max(0, std::min(255, (int)((h - minY) * ext)));
                    data[y * tsz * tsz + z * tsz + x] = 1;
                }
            }
        }

        m_rledata = RleEncode(data);
    }

    float OctTile::GetGroundPos(const Point2f& pt) const
    {
        if (m_rledata.size() == 0)
            return NAN;

        AABoxf bboxoct = m_l.GetBBox();
        float extent = TerrainTile::SquarePtsCt / (bboxoct.mMax[0] - bboxoct.mMin[0]);
        float invextent = (bboxoct.mMax[0] - bboxoct.mMin[0]) / TerrainTile::SquarePtsCt;
        int x = (pt[0] - bboxoct.mMin[0]) * extent;
        int z = (pt[1] - bboxoct.mMin[2]) * extent;

        x = std::min(TerrainTile::SquarePtsCt - 1,
            std::max(0, x));
        z = std::min(TerrainTile::SquarePtsCt - 1,
            std::max(0, z));

        std::vector<byte> data = RleDecode(m_rledata);
        const int tsz = TerrainTile::SquarePtsCt;
        for (int y = 255; y >= 0; --y)
        {
            if (data[y * tsz * tsz + z * tsz + x] > 0)
            {
                return y * invextent + bboxoct.mMin[1];
            }
        }
        return NAN;
    }

    void OctTile::BackgroundLoad(World* pWorld)
    {
        if (m_readyState == 0)
        {
            m_readyState = 1;
            bool success = false;
            std::string strval;
            if (pWorld->Level().GetOctChunk(m_l, &strval))
            {
                m_rledata.resize(strval.size());
                memcpy(m_rledata.data(), strval.data(), strval.size());
                LoadVB();
                m_readyState = 3;
            }

        }
        if (m_readyState == 1)
        {
            if (m_terrainTile != nullptr)
                m_readyState = 2;
            else if (pWorld->TerrainTileSelection().RequestTile(m_l, pWorld, m_terrainTile))
                m_readyState = 2;
        }

        if (m_readyState == 2)
        {
            LoadTerrainData();
            pWorld->Level().WriteOctChunk(m_l, (const char*)m_rledata.data(), m_rledata.size());
            LoadVB();
            m_readyState = 3;
        }
    }

    extern Loc g_hitLoc;
    bool g_showOctBoxes = false;
    void OctTile::Draw(DrawContext& ctx)
    {
        nOctTilesTotal++;
        nOctTilesDrawn++;
        
        if (ctx.m_nearfarpassIdx == 0 && m_farDist < ctx.m_nearfar[1])
            return;
        if (ctx.m_nearfarpassIdx == 1 && m_nearDist > ctx.m_nearfar[1])
            return;
            
        if (!bgfx::isValid(m_uparams))
        {
            m_uparams = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, 1);
        }

        if (!g_showOctBoxes && m_readyState >= 3 && m_cubeList != nullptr)
        {
            m_cubeList->Use();
            Vec4f color = (ctx.m_nearfarpassIdx == 0) ? Vec4f(0.4f, 0.2f, 0.2f, 1) : Vec4f(0.2f, 0.2f, 0.4f, 1);
            bgfx::setUniform(m_uparams, &color, 1);
            Matrix44f m;
            identity(m);
            bgfx::setTransform(m.getData());
            // Set vertex and index buffer.
            bgfx::setVertexBuffer(0, m_cubeList->vbh);
            bgfx::setIndexBuffer(m_cubeList->ibh);
            uint64_t state = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_MSAA
                | BGFX_STATE_BLEND_ALPHA;
            // Set render states.l
            bgfx::setState(state);
            bgfx::submit(ctx.m_curviewIdx, ctx.m_pgm);

            if (m_rawdata.size() > 0 && m_lastUsedRawData++ > 60)
            {
                m_rawdata = std::vector<byte>();
            }
        }
        else if (g_showOctBoxes)
        {
            if (!sBboxshader.isValid())
                sBboxshader = Engine::Inst().LoadShader("vs_cubes.bin", "fs_bbox.bin");
            Cube::init();
            AABoxf bbox = m_l.GetBBox();
            float scl = (bbox.mMax[0] - bbox.mMin[0]) * 0.45f;
            Point3f off = (bbox.mMax + bbox.mMin) * 0.5f;
            Matrix44f m = makeTrans<Matrix44f>(off) *
                makeScale<Matrix44f>(scl);
            bgfx::setTransform(m.getData());
            Vec4f color = m_l == g_hitLoc ? Vec4f(1.0f, 0.0f, 1.0f, 1.0f) : Vec4f(0.0f, 1.0f, 1.0f, 1.0f);
            bgfx::setUniform(m_uparams, &color, 1);
            uint64_t state = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_MSAA
                | BGFX_STATE_BLEND_ALPHA;
            // Set render states.l
            bgfx::setState(state);
            bgfx::setVertexBuffer(0, Cube::vbh);
            bgfx::setIndexBuffer(Cube::ibh);
            bgfx::submit(ctx.m_curviewIdx, sBboxshader);
        }
    }

    void OctTile::Decomission()
    {
        m_readyState = 0;
    }

    bool OctTile::Intersect(const Point3f& pt0, const Point3f& pt1, Vec3i & hitpt)
    {
        if (m_rledata.size() == 0)
            return false;

        AABoxf aabb = m_l.GetBBox();
        Vec3f extents = aabb.mMax - aabb.mMin;
        float mul = TerrainTile::SquarePtsCt / extents[0];
        Vec3f c0 = (pt0 - aabb.mMin) * mul;
        Vec3f c1 = (pt1 - aabb.mMin) * mul;
        std::vector<Vec3i> pts;
        if (m_rawdata.size() == 0)
        {
            m_rawdata = RleDecode(m_rledata);
        }
        m_lastUsedRawData = 0;
        hitpt = FindHit(m_rawdata, Vec3i(c0[0], c0[1], c0[2]), Vec3i(c1[0], c1[1], c1[2]));
        if (hitpt[0] > 0)
            return true;
        return false;
    }

    OctTile::~OctTile()
    {
        Decomission();
    }


#define checkhit(x, y, z) if (data[y * tsz * tsz + z * tsz + x] > 0) return Vec3i(x, y, z);
#define R(x) std::max(0, std::min(TerrainTile::SquarePtsCt - 1, x))

    Vec3i OctTile::FindHit(const std::vector<byte>& data, const Vec3i pt1, const Vec3i pt2)
    {
        const int tsz = TerrainTile::SquarePtsCt;

        int x1 = R(pt1[0]), y1 = R(pt1[1]), z1 = R(pt1[2]);
        int x2 = R(pt2[0]), y2 = R(pt2[1]), z2 = R(pt2[2]);

        checkhit(x1, y1, z1);

        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int dz = abs(z2 - z1);
        int xs, ys, zs;
        if (x2 > x1)
            xs = 1;
        else
            xs = -1;

        if (y2 > y1)
            ys = 1;
        else
            ys = -1;
        if (z2 > z1)
            zs = 1;
        else
            zs = -1;

        // Driving axis is X - axis"
        if (dx >= dy && dx >= dz)
        {
            int p1 = 2 * dy - dx;
            int p2 = 2 * dz - dx;
            while (x1 != x2)
            {
                x1 += xs;
                if (p1 >= 0)
                {
                    y1 += ys;
                    p1 -= 2 * dx;
                }
                if (p2 >= 0)
                {
                    z1 += zs;
                    p2 -= 2 * dx;
                }
                p1 += 2 * dy;
                p2 += 2 * dz;
                checkhit(x1, y1, z1);
            }
        }
        // Driving axis is Y - axis"
        else if (dy >= dx && dy >= dz)
        {
            int p1 = 2 * dx - dy;
            int p2 = 2 * dz - dy;
            while (y1 != y2) {
                y1 += ys;
                if (p1 >= 0)
                {
                    x1 += xs;
                    p1 -= 2 * dy;
                }
                if (p2 >= 0) {
                    z1 += zs;
                    p2 -= 2 * dy;
                }
                p1 += 2 * dx;
                p2 += 2 * dz;
                checkhit(x1, y1, z1);
            }
        }

        // Driving axis is Z - axis"
        else
        {
            int p1 = 2 * dy - dz;
            int p2 = 2 * dx - dz;
            while (z1 != z2) {
                z1 += zs;
                if (p1 >= 0) {
                    y1 += ys;
                    p1 -= 2 * dz;
                }
                if (p2 >= 0) {
                    x1 += xs;
                    p2 -= 2 * dz;
                }
                p1 += 2 * dy;
                p2 += 2 * dx;
                checkhit(x1, y1, z1);
            }
        }
        return Vec3i(-1, -1, -1);
    }

    void TargetCube::Initialize(DrawContext& nvg)
    {
        m_shader = Engine::Inst().LoadShader("vs_cubes.bin", "fs_targetcube.bin");
    }

    void TargetCube::Draw(DrawContext& ctx)
    {
        Cube::init();
        Matrix44f m = ctx.m_mat* CalcMat();
        bgfx::setTransform(m.getData());
        // Set vertex and index buffer.
        bgfx::setVertexBuffer(0, Cube::vbh);
        bgfx::setIndexBuffer(Cube::ibh);
        uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA;
        // Set render states.l
        bgfx::setState(state);
        bgfx::submit(ctx.m_curviewIdx, m_shader);
    }

}
