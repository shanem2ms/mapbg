#include "StdIncludes.h"
#include "OctTileSelection.h"
#include "Application.h"
#include "Engine.h"
#include "TerrainTileSelection.h"
#include <numeric>
#include "Mesh.h"
#include "gmtl/PlaneOps.h"
#include <sstream>
#define NOMINMAX
#ifdef _WIN32
#include <Windows.h>
#endif




using namespace gmtl;

namespace sam
{
    std::atomic<size_t> OctTileSelection::sNumTiles = 0;

    class FrustumTiles
    {
    public:
        static void Get(Camera& cam, std::vector<Loc>& locs, float pixelDist, int maxlod, const AABoxf& playerBounds)
        {
            Frustumf viewFrust = cam.GetFrustum();
            Matrix44f viewproj = cam.PerspectiveMatrix() * cam.ViewMatrix();
            Vec3f ctr = FrustumCenter(viewproj);


            Vec3f chkpos(floorf(ctr[0]), floorf(ctr[1]), floorf(ctr[2]));
            GetLocsInView(locs, Loc(0, 0, 0, 0), viewFrust, viewproj, pixelDist, maxlod, playerBounds);
        }

    private:
        enum class ContainmentType
        {
            Disjoint = 0,
            Contains = 1,
            Intersects = 2
        };

        static Vec3f FrustumCenter(Matrix44f viewproj)
        {
            Matrix44f mInv;
            invert(viewproj);
            Vec4f inpt4;
            xform(inpt4, mInv, Vec4f(0, 0, 0.5f, 1.0f));
            inpt4 /= inpt4[3];
            return Vec3f(inpt4[0], inpt4[1], inpt4[2]);
        }


        template< class DATA_TYPE >
        DATA_TYPE static abs_distance(const Plane<DATA_TYPE>& plane, const Point<DATA_TYPE, 3>& pt)
        {
            return (dot(plane.mNorm, static_cast<Vec<DATA_TYPE, 3>>(pt)) + plane.mOffset);
        }


        static ContainmentType Contains(Frustumf f, AABoxf box)
        {
            ContainmentType result = ContainmentType::Contains;
            for (int i = 0; i < 6; i++)
            {
                Planef plane = f.mPlanes[i];

                // Approach: http://zach.in.tu-clausthal.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html

                Point3f positive = Point3f(box.mMin[0], box.mMin[1], box.mMin[2]);
                Point3f negative = Point3f(box.mMax[0], box.mMax[1], box.mMax[2]);

                if (plane.mNorm[0] >= 0)
                {
                    positive[0] = box.mMax[0];
                    negative[0] = box.mMin[0];
                }
                if (plane.mNorm[1] >= 0)
                {
                    positive[1] = box.mMax[1];
                    negative[1] = box.mMin[1];
                }
                if (plane.mNorm[2] >= 0)
                {
                    positive[2] = box.mMax[2];
                    negative[2] = box.mMin[2];
                }

                // If the positive vertex is outside (behind plane), the box is disjoint.
                float positiveDistance = abs_distance(plane, positive);
                if (positiveDistance < 0)
                {
                    return ContainmentType::Disjoint;
                }

                // If the negative vertex is outside (behind plane), the box is intersecting.
                // Because the above check failed, the positive vertex is in front of the plane,
                // and the negative vertex is behind. Thus, the box is intersecting this plane.
                float negativeDistance = abs_distance(plane, negative);
                if (negativeDistance < 0)
                {
                    result = ContainmentType::Intersects;
                }
            }

            return result;
        }


        static inline void GetCorners(const AABoxf& box, Point3f pts[8])
        {
            const Point3f& l = box.mMin;
            const Point3f& u = box.mMax;
            pts[0] = Point3f(l[0], l[1], l[2]);
            pts[1] = Point3f(l[0], l[1], u[2]);
            pts[2] = Point3f(l[0], u[1], l[2]);
            pts[3] = Point3f(l[0], u[1], u[2]);
            pts[4] = Point3f(u[0], l[1], l[2]);
            pts[5] = Point3f(u[0], l[1], u[2]);
            pts[6] = Point3f(u[0], u[1], l[2]);
            pts[7] = Point3f(u[0], u[1], u[2]);
        }

        static void GetLocsInView(std::vector<Loc>& locs, const Loc& curLoc,
            const Frustumf& f, const Matrix44f& viewProj, float pixelDist, int maxlod, const AABoxf& playerBounds)
        {
            AABoxf aabox = curLoc.GetBBox();
            Point3f ptinc = (aabox.mMax - aabox.mMin) * 0.2f;
            Point3f c[8];
            GetCorners(aabox, c);
            float avgdist = 0;
            for (int idx = 0; idx < 8; ++idx)
            {
                Point4f pt0(c[idx][0], c[idx][1], c[idx][2], 1);
                Point4f ppt0;
                xform(ppt0, viewProj, pt0);
                ppt0 /= ppt0[3];

                Point4f pt1(c[idx][0] + ptinc[0], c[idx][1] + ptinc[1], c[idx][2] + ptinc[2], 1);
                Point4f ppt1;
                xform(ppt1, viewProj, pt1);
                ppt1 /= ppt1[3];

                float dist = length(Vec4f(ppt1 - ppt0));
                avgdist += dist;
            }

            if (curLoc.m_l > 1 && (avgdist < pixelDist || curLoc.m_l >= maxlod))
            {
                locs.push_back(curLoc);
                return;
            }

            std::vector<Loc> children = curLoc.GetChildren();
            for (const Loc& childLoc : children)
            {
                AABoxf cbox = childLoc.GetBBox();
                ContainmentType res = Contains(f, cbox);
                bool intersectsPlayer = intersect(playerBounds, cbox);
                if (res != ContainmentType::Disjoint || intersectsPlayer)
                {
                    GetLocsInView(locs, childLoc, f, viewProj, pixelDist, maxlod, playerBounds);
                }
            }
        }
    };

    OctTileSelection::OctTileSelection()
    {
        m_nearfarmidsq[0] = 0.1f;
        m_nearfarmidsq[1] = 25.0f;
        m_nearfarmidsq[2] = 100.0f;
        m_terrainSelection = std::make_unique<TerrainTileSelection>();
    }

    extern int g_maxTileLod;
    void OctTileSelection::Update(Engine& e, DrawContext& ctx, const AABoxf& playerBounds)
    {
        auto oldTiles = m_activeTiles;
        m_activeTiles.clear();
        auto& cam = e.Cam();
        Camera::Fly fly = cam.GetFly();

        std::vector<Loc> locs;
        FrustumTiles::Get(e.Cam(), locs, 10.0f, g_maxTileLod, playerBounds);

        std::sort(locs.begin(), locs.end());        

        for (const auto& l : locs)
        {
            auto itSq = m_tiles.find(l);
            if (itSq == m_tiles.end())
            {
                std::shared_ptr<OctTile> sq = std::make_shared<OctTile>(l);
                { // Init OctTile
                    float sx = l.m_x;
                    float sy = l.m_z;
                    
                    Point3f pos = l.GetCenter();
                    sq->SetOffset(pos);
                    float s = l.GetExtent() * 0.5f;                    
                    sq->SetScale(Vec3f(s, s, s));
                }
                m_tiles.insert(std::make_pair(l, sq));
                sNumTiles++;
            }
            m_activeTiles.insert(l);
        }

        m_terrainSelection->SelectTiles(locs, ctx.m_pWorld);

        for (auto loc : oldTiles)
        {
            if (m_activeTiles.find(loc) == m_activeTiles.end())
            {
                m_tiles.erase(loc);
                sNumTiles--;
            }
        }

        auto terrainTiles = m_terrainSelection->Tiles();
        for (auto sqPair : m_activeTiles)
        {
            auto itSq = m_tiles.find(sqPair);
            Loc tloc = itSq->first;
            tloc.m_y = 0;
            auto itTerrainTile = terrainTiles.find(tloc);
            if (itTerrainTile != terrainTiles.end())
            {
                itTerrainTile->second->SetLastUseFrame(ctx.m_frameIdx);
                itSq->second->SetTerrainTile(itTerrainTile->second);
            }
            AABoxf box = sqPair.GetBBox();
            const Point3f& m0 = box.mMin;
            const Point3f& m1 = box.mMax;
            Point3f pts[9] = {
                Point3f(m0[0], m0[1], m0[2]),
                Point3f(m0[0], m0[1], m1[2]),
                Point3f(m0[0], m1[1], m0[2]),
                Point3f(m0[0], m1[1], m1[2]),
                Point3f(m1[0], m0[1], m0[2]),
                Point3f(m1[0], m0[1], m1[2]),
                Point3f(m1[0], m1[1], m0[2]),
                Point3f(m1[0], m1[1], m1[2])
            };
            float minlen = std::numeric_limits<float>::max();
            float maxlen = 0;
            for (int i = 0; i < 8; ++i)
            {
                float l = lengthSquared(Vec3f(fly.pos - pts[i]));
                minlen = std::min(minlen, l);
                maxlen = std::max(maxlen, l);
            }
            itSq->second->nearDistSq = minlen;
            itSq->second->farDistSq = maxlen;
            itSq->second->distFromCam = (minlen + maxlen) * 0.5f;
        }
    }

    int g_nearTiles;
    int g_farTiles;

    void OctTileSelection::AddTilesToGroup(std::shared_ptr<SceneGroup> grp)
    {
        std::vector<std::shared_ptr<OctTile>> tiles;
        for (auto sqPair : m_activeTiles)
        {
            auto itSq = m_tiles.find(sqPair);
            if (!itSq->second->IsEmpty())
                tiles.push_back(itSq->second);
        }

        if (tiles.size() == 0)
            return;

        std::sort(tiles.begin(), tiles.end(), [](auto& t1, auto& t2) { return t1->distFromCam > t2->distFromCam;  });
        

        float splitdistsq = tiles[tiles.size() / 2]->distFromCam;
        float neardistsq = std::numeric_limits<float>::max();
        float fardistsq = 0;
        for (auto& t : tiles)
        {
//            splitdistsq += t->nearDistSq;
            neardistsq = std::min(neardistsq, t->nearDistSq);
            fardistsq = std::max(fardistsq, t->farDistSq);
        }

        m_nearfarmidsq[0] = pow(0.002, 2);
        m_nearfarmidsq[1] = splitdistsq;
        m_nearfarmidsq[2] = fardistsq;
        int nearCt = 0;
        int farCt = 0;
        for (auto& t : tiles)
        {
            if (t->nearDistSq < splitdistsq)
                nearCt++;
            if (t->farDistSq > splitdistsq)
                farCt++;
        }
        g_nearTiles = nearCt;
        g_farTiles = farCt;

        for (auto& t : tiles)
        {
            grp->AddItem(t);
        }

    }

    void OctTileSelection::GetNearFarMidDist(float nearfarmidsq[3])
    {
        memcpy(nearfarmidsq, m_nearfarmidsq, sizeof(float) * 3);
    }

    float OctTileSelection::GetGroundHeight(const Point3f& pt)
    {
        int tx = (int)floor(pt[0]);
        int tz = (int)floor(pt[2]);
        Loc queryLoc(tx, 0, tz);

        static float headHeight = 0.04f;
        auto itCamTile = m_tiles.find(queryLoc);
        if (itCamTile != m_tiles.end())
        {
            return 0;// itCamTile->second->GetGroundHeight(pt) + headHeight;
        }
    }


    std::shared_ptr<OctTile> OctTileSelection::TileFromPos(const Point3f& pos)
    {
        for (auto& pair : m_tiles)
        {
            if (intersect(pair.first.GetBBox(), pos))
            {
                return pair.second;
            }
        }

        return nullptr;
    }

    OctTileSelection::~OctTileSelection()
    {

    }

}