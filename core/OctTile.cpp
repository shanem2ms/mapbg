#include "StdIncludes.h"
#include "World.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "OctTile.h"
#include "TerrainTile.h"
#include "gmtl/Intersection.h"
#include "leveldb/db.h"
#define NOMINMAX

namespace sam
{

    OctTile::OctTile(const Loc& l) : m_image(-1), m_l(l), m_needRebuild(true),
        m_buildFrame(0),
        m_dataready(false)
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
        int x = (pt[0] - bboxoct.mMin[0])* extent;
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

    void OctTile::Draw(DrawContext& ctx)
    {
        nOctTilesTotal++;
        if (m_terrainTile == nullptr || !m_terrainTile->IsDataReady())
            return;
        nOctTilesDrawn++;
        if (ctx.m_nearfarpassIdx == 0 && farDistSq < ctx.m_nearfar[1])
            return;
        if (ctx.m_nearfarpassIdx == 1 && nearDistSq > ctx.m_nearfar[1])
            return;
        
        if (m_needRebuild)
        {
            leveldb::Slice key((const char*)&m_l, sizeof(m_l));
            leveldb::DB* db = ctx.m_pWorld->Db();
            std::string strval;
            leveldb::Status status = db->Get(leveldb::ReadOptions(), key, &strval);
            if (status.ok())
            {
                m_rledata.resize(strval.size());
                memcpy(m_rledata.data(), strval.data(), strval.size());
            }
            else
            {
                LoadTerrainData();
                if (m_rledata.size() > 0)
                {
                    leveldb::Slice val((const char*)m_rledata.data(), m_rledata.size());
                    leveldb::Status status = db->Put(leveldb::WriteOptions(), key, val);
                }
            }
            if (m_rledata.size() > 0)
                LoadVB();

            m_needRebuild = false;
        }

        if (m_cubeList == nullptr)
            return;

        if (!bgfx::isValid(m_uparams))
        {
            m_uparams = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, 1);
        }

        static Vec3f c[] = {
            Vec3f(1, 0, 0),
            Vec3f(0.6f, 0.4f, 0),
            Vec3f(1.0f, 1.0f, 0),
            Vec3f(0, 1.0f, 0),
            Vec3f(0, 0, 1.0f),
            Vec3f(1.0f, 0, 1.0f)
        };
        int t = m_l.m_l;
        Vec3f cc = c[t % 6];
        Vec4f color(cc[0], cc[1], cc[2],
            1);
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
    }

    void OctTile::Decomission()
    {
        m_needRebuild = true;
    }

    OctTile::~OctTile()
    {
        Decomission();
    }
}
