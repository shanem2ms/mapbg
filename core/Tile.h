#pragma once

#include <map>
#include <set>
#include "SceneItem.h"
namespace sam
{

    struct Loc
    {
        Loc(int x, int y, int z) :
            m_x(x),
            m_y(y),
            m_z(z),
            m_l(0) {}

        int m_x;
        int m_y;
        int m_z;
        int m_l;

        bool operator < (const Loc& rhs)
        {
            if (m_x != rhs.m_x)
                return m_x < rhs.m_x;
            if (m_y != rhs.m_y)
                return m_y < rhs.m_y;
            if (m_z != rhs.m_z)
                return m_z < rhs.m_z;
            if (m_l != rhs.m_l)
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
    };

    class Tile : public SceneItem
    {
        static const int SquarePtsCt = 256;
        static const int OverlapPtsCt = 64;
        static const int TotalPtsCt = SquarePtsCt + OverlapPtsCt * 2;

        float m_squareSize;
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

    public:
        void Draw(DrawContext& ctx) override;
        Tile(const Loc& l);
        ~Tile();
        void SetSquareSize(float squareSize)
        {
            m_squareSize = squareSize;
        }

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
}