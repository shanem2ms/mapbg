#include "TileSelection.h"
#include "Application.h"
#include "Engine.h"
#include "HSLColor.h"
#include "SimplexNoise/SimplexNoise.h"
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

    class FrustumTiles
    {
    public:
        static void Get(Camera& cam, std::vector<Loc>& locs, float pixelDist)
        {
            Frustumf viewFrust = cam.GetFrustum();
            Matrix44f viewproj = cam.PerspectiveMatrix() * cam.ViewMatrix();
            Vec3f ctr = FrustumCenter(viewproj);


            Vec3f chkpos(floorf(ctr[0]), floorf(ctr[1]), floorf(ctr[2]));
            GetBBoxes(locs, Loc(0, 0, 0, 0), viewFrust, viewproj, pixelDist);
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

        static void GetBBoxes(std::vector<Loc>& locs, const Loc& curLoc,
            const Frustumf& f, const Matrix44f& viewProj, float pixelDist)
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

            if (curLoc.m_l > 4 && avgdist < pixelDist)
            {
                locs.push_back(curLoc);
                return;
            }

            std::vector<Loc> children = curLoc.GetChildren();
            for (const Loc& childLoc : children)
            {
                AABoxf cbox = childLoc.GetBBox();
                ContainmentType res = Contains(f, cbox);
                if (res != ContainmentType::Disjoint)
                {
                    GetBBoxes(locs, childLoc, f, viewProj, pixelDist);
                }
            }
        }
    };

    TileSelection::TileSelection()
    {
    }

    void TileSelection::Update(Engine& e, DrawContext& ctx)
    {
        auto oldTiles = m_activeTiles;
        m_activeTiles.clear();
        auto& cam = e.Cam();
        Camera::Fly fly = cam.GetFly();

        int tx = (int)floor(fly.pos[0]);
        int tz = (int)floor(fly.pos[2]);

        std::vector<Loc> locs;
        FrustumTiles::Get(e.Cam(), locs, 10.0f);

        std::sort(locs.begin(), locs.end());

        std::vector<Loc> &allLocs = locs;
        std::ostringstream ss;
        ss << allLocs.size() << std::endl;
        OutputDebugStringA(ss.str().c_str());


        for (const auto& l : allLocs)
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
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        auto itNeightborSq = m_tiles.find(Loc(l.m_x + dx, 0, l.m_z + dy));
                        if (itNeightborSq == m_tiles.end())
                            continue;
                    }
                }
            }
            m_activeTiles.insert(l);
        }

        for (auto loc : oldTiles)
        {
            if (m_activeTiles.find(loc) == m_activeTiles.end())
            {
                m_tiles[loc]->Decomission();
            }
        }

        for (auto sqPair : m_activeTiles)
        {
            auto itSq = m_tiles.find(sqPair);
            itSq->second->distFromCam = lengthSquared(Vec3f(fly.pos - sqPair.GetCenter()));
        }
    }

    void TileSelection::AddTilesToGroup(std::shared_ptr<SceneGroup> grp)
    {
        std::vector<std::shared_ptr<OctTile>> tiles;
        for (auto sqPair : m_activeTiles)
        {
            auto itSq = m_tiles.find(sqPair);
            tiles.push_back(itSq->second);
        }

        std::sort(tiles.begin(), tiles.end(), [](auto& t1, auto& t2) { return t1->distFromCam > t2->distFromCam;  });
        for (auto& t : tiles)
        {
            grp->AddItem(t);
        }

    }

    float TileSelection::GetGroundHeight(const Point3f& pt)
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



    TileSelection::~TileSelection()
    {

    }

}