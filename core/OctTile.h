#pragma once

#include <map>
#include <set>
#include "SceneItem.h"
struct CubeList;
namespace sam
{
    class TerrainTile;

    struct Loc
    {
        Loc(int x, int y, int z) :
            Loc(x,y,z,8) {}

        Loc(int x, int y, int z, int l) :
            m_x(x),
            m_y(y),
            m_z(z),
            m_l(l) {}

        int m_x;
        int m_y;
        int m_z;
        int m_l;
        static const int lsize = 8;
        static const int ox = -(1 << (lsize - 1));
        static const int oy = -(1 << (lsize - 1));
        static const int oz = -(1 << (lsize - 1));

        static constexpr uint64_t GetLodOffset(int l)
        {
            if (l == 0)
                return 0;
            else
                return (1 << l) * (1 << l) * (1 << l) +
                GetLodOffset(l - 1);
        }

        static constexpr uint64_t GetTileIndex(const Loc &l)
        {
            return GetLodOffset(l.m_l) +
            l.m_x* (1 << lsize)* (1 << lsize) +
                l.m_y * (1 << lsize) +
                l.m_z;
        }

        static constexpr void GetLocFromIndex(Loc &loc, uint64_t index)
        {
            int l = 0;
            while (index > (1 << l) * (1 << l) * (1 << l))
            {
                index -= (1 << l) * (1 << l) * (1 << l);
                l++;
            }

            loc.m_l = l;
            loc.m_x = index / ((1 << l) * (1 << l));
            index -= loc.m_x * ((1 << l) * (1 << l));
            loc.m_y = index / (1 << l);
            index -= (1 << l);
            loc.m_z = index;
        }

        bool operator < (const Loc& rhs)
        {
            if (m_l != rhs.m_l)
                return m_l < rhs.m_l;
            if (m_x != rhs.m_x)
                return m_x < rhs.m_x;
            if (m_y != rhs.m_y)
                return m_y < rhs.m_y;
            return m_z < rhs.m_z;
        }

        bool operator == (const Loc& rhs)
        {
            if (m_x != rhs.m_x)
                return false;
            if (m_y != rhs.m_y)
                return false;
            if (m_z != rhs.m_z)
                return false;
            if (m_l != rhs.m_l)
                return false;

            return true;
        }

        float GetExtent() const { return powf(2.0, lsize - m_l); }

        AABoxf GetBBox() const {
            float dist = GetExtent();
            return AABoxf(Point3f(ox + m_x * dist, oy + m_y * dist, oz + m_z * dist), Point3f(ox + (m_x + 1) * dist, oy + (m_y + 1) * dist, oz + (m_z + 1) * dist));
        }

        Point3f GetCenter() const
        {
            float dist = GetExtent();
            return Point3f(ox + (m_x + 0.5f) * dist, oy + (m_y + 0.5f) * dist, oz + (m_z + 0.5f) * dist);
        }

        std::vector<Loc> GetChildren() const
        {
            return std::vector<Loc>{
                        Loc(m_x * 2, m_y * 2, m_z * 2, m_l + 1),
                        Loc(m_x * 2, m_y * 2, m_z * 2 + 1, m_l + 1),
                        Loc(m_x * 2, m_y * 2 + 1, m_z * 2, m_l + 1),
                        Loc(m_x * 2, m_y * 2 + 1, m_z * 2 + 1, m_l + 1),
                        Loc(m_x * 2 + 1, m_y * 2, m_z * 2, m_l + 1),
                        Loc(m_x * 2 + 1, m_y * 2, m_z * 2 + 1, m_l + 1),
                        Loc(m_x * 2 + 1, m_y * 2 + 1, m_z * 2, m_l + 1),
                        Loc(m_x * 2 + 1, m_y * 2 + 1, m_z * 2 + 1, m_l + 1)
                };
        }

        bool IsGroundLoc() const
        {            
            return m_y == (1 << (m_l - 1));
        }

        Loc GetGroundLoc() const
        {
            return Loc(m_x, (1 << (m_l - 1)), m_z, m_l);
        }

        Loc Parent() const
        {
            return Loc(m_x >> 1, m_y >> 1, m_z >> 1, m_l - 1);
        }

        Loc ParentAtLevel(int l) const
        {
            int d = m_l - l;
            return Loc(m_x >> d, m_y >> d, m_z >> d, l);
        }

        Loc GetLocal(const Loc& parent)
        {
            int d = m_l - parent.m_l;
            return Loc(
                m_x - (parent.m_x << d),
                m_y - (parent.m_y << d),
                m_z - (parent.m_z << d),
                d);
        }

        Loc GetChild(const Loc& local)
        {
            return Loc(
                (m_x << local.m_l) + local.m_x,
                (m_y << local.m_l) + local.m_y,
                (m_z << local.m_l) + local.m_z,
                m_l + local.m_l);
        }

    };

    inline std::ostream& operator<<(std::ostream& os, const Loc& loc)
    {
        os << "[" << loc.m_l << ", " << loc.m_x << ", " << loc.m_y << ", " << loc.m_z << "]";
        return os;
    }
    class OctTile : public SceneItem
    {
        int m_image;
        Vec2f m_vals;
        Loc m_l;
        bool m_needRebuild;
        Vec2f m_maxdh;
        Vec2f m_mindh;
        int m_texpingpong;
        int m_buildFrame;
        bool m_dataready;
        bgfxh<bgfx::UniformHandle> m_uparams;
        std::shared_ptr<TerrainTile> m_terrainTile;
        std::shared_ptr<CubeList> m_cubeList;
        std::vector<byte> m_rledata;
    public:
        float nearDistSq;
        float farDistSq;
        float distFromCam;
    public:
        void Draw(DrawContext& ctx) override;
        OctTile(const Loc& l);
        ~OctTile();

        bool IsEmpty() const { return m_terrainTile == nullptr;  }

        void SetTerrainTile(std::shared_ptr<TerrainTile> terrainTile)
        {
            m_terrainTile = terrainTile;
        }

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
    private:
        static std::vector<byte> RleEncode(const std::vector<byte> data);
        static std::vector<byte> RleDecode(const std::vector<byte> data);
        void LoadTerrainData();
    };

    inline bool operator < (const Loc& lhs, const Loc& rhs)
    {
        if (lhs.m_y != rhs.m_y)
            return lhs.m_y < rhs.m_y;
        if (lhs.m_x != rhs.m_x)
            return lhs.m_x < rhs.m_x;
        if (lhs.m_l != rhs.m_l)
            return lhs.m_l < rhs.m_l;
        if (lhs.m_z != rhs.m_z)
            return lhs.m_z < rhs.m_z;

        return false;
    }
}