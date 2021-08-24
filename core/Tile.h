#pragma once

#include <map>
#include <set>
#include "SceneItem.h"
namespace sam
{

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

        bool operator < (const Loc& rhs)
        {
            if (m_x != rhs.m_x)
                return m_x < rhs.m_x;
            if (m_y != rhs.m_y)
                return m_y < rhs.m_y;
            if (m_z != rhs.m_z)
                return m_z < rhs.m_z;
            return m_l < rhs.m_l;
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

    class Tile : public SceneItem
    {
        static const int SquarePtsCt = 256;
        static const int OverlapPtsCt = 64;
        static const int TotalPtsCt = SquarePtsCt + OverlapPtsCt * 2;

        int m_image;
        Vec2f m_vals;
        std::weak_ptr<Tile> m_neighbors[9];
        Loc m_l;
        bool m_needRecalc;
        float m_pts[TotalPtsCt * TotalPtsCt];
        Vec2f m_maxdh;
        Vec2f m_mindh;
        bgfx::TextureHandle m_tex[2];
        bgfx::TextureHandle m_terrain;
        bgfx::TextureHandle m_rbTex;
        int m_texpingpong;
        int m_buildFrame;
        bool m_dataready;
        std::vector<float> m_heightData;
        bgfx::UniformHandle m_uparams;

    public:
        float distFromCam;
    public:
        void Draw(DrawContext& ctx) override;
        Tile(const Loc& l);
        ~Tile();

        const float* Pts() const { return m_pts; }
        void SetImage(int image)
        {
            m_image = image;
        }
        gmtl::AABoxf GetBounds() const override;
        void SetVals(const Vec2f& v)
        {
            m_vals = v;
        }
        void SetNeighbor(int dx, int dy, std::weak_ptr<Tile> sq);
        void Decomission();

        float GetGroundHeight(const gmtl::Point3f& pt) const;

    private:
        void NoiseGen();
        void ProceduralBuild(DrawContext& ctx);
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