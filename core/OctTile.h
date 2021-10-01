#pragma once

#include <map>
#include <set>
#include "SceneItem.h"
#include "Loc.h"

struct VoxCube;

namespace sam
{
    class TerrainTile;

    class OctTile : public SceneItem
    {
        
        int m_image;
        Vec2f m_vals;
        Loc m_l;
        Vec2f m_maxdh;
        Vec2f m_mindh;
        int m_texpingpong;
        int m_buildFrame;
        int m_readyState;
        bgfxh<bgfx::UniformHandle> m_uparams;
        std::shared_ptr<TerrainTile> m_terrainTile;
        std::shared_ptr<VoxCube> m_voxelinst;
        std::vector<byte> m_rledata;
        std::vector<byte> m_rawdata;
        int m_lastUsedRawData;
        float m_intersects;
    public:
        float m_nearDist;
        float m_farDist;
        float distFromCam;
    public:
        void Draw(DrawContext& ctx) override;
        OctTile(const Loc& l);
        ~OctTile();

        void BackgroundLoad(World *pWorld);
        bool IsEmpty() const { return m_rledata.size() == 0;  }

        void SetIntersects(float i)
        { m_intersects = i; }

        void SetImage(int image)
        {
            m_image = image;
        }
        gmtl::AABoxf GetBounds() const override;
        void SetVals(const Vec2f& v)
        {
            m_vals = v;
        }
        void Decomission();
        void LoadVB();
        float GetGroundPos(const Point2f& pt) const;
        bool Intersect(const Point3f& pt0, const Point3f& pt1, Vec3i &hitpt);
        static Vec3i FindHit(const std::vector<byte> &data, const Vec3i p1, const Vec3i p2);
        int GetReadyState() const
        { return m_readyState; }
    private:
        static std::vector<byte> RleEncode(const std::vector<byte> data);
        static std::vector<byte> RleDecode(const std::vector<byte> data);
        void LoadTerrainData();
    };

    class TargetCube : public SceneItem
    {
        bgfx::ProgramHandle m_shader;
        void Initialize(DrawContext& nvg) override;
        void Draw(DrawContext& ctx) override;
    };
}