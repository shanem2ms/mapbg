#pragma once

#include <map>
#include <set>
#include "SceneItem.h"

class SimplexNoise;
namespace sam
{

    struct DrawContext;
    class Engine;
    class Touch;
    class Board
    {
    public:
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

        static const int SquarePtsCt = 256;
        static const int OverlapPtsCt = 64;
        static const int TotalPtsCt = SquarePtsCt + OverlapPtsCt * 2;

        class Square : public SceneItem
        {
            float m_squareSize;
            int m_image;
            Vec2f m_vals;
            std::weak_ptr<Square> m_neighbors[9];
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
            Square(const Loc& l);
            ~Square();
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
            void SetNeighbor(int dx, int dy, std::weak_ptr<Square> sq);
            void Decomission();

            float GetGroundHeight(const gmtl::Point3f& pt) const;

        private:
            void NoiseGen();
            void ProceduralBuild(DrawContext& ctx);
        };

        std::map<Loc, std::shared_ptr<Square>> m_squares;
        std::set<Loc> m_activeSquares;

        float m_squareSize;
        int m_width;
        int m_height;
        gmtl::Point3f m_camVel;
        float m_tiltVel;

        std::shared_ptr<SceneGroup> m_boardGroup;
        std::shared_ptr<Touch> m_activeTouch;
        int m_currentTool;

    public:
        void Layout(int w, int h);
        Board();
        ~Board();
        void Update(Engine& engine, DrawContext& ctx);
        void TouchDown(float x, float y, int touchId);
        void TouchDrag(float x, float y, int touchId);
        void TouchUp(int touchId);
        void KeyDown(int k);
        void KeyUp(int k);
    };

    inline bool operator < (const Board::Loc& lhs, const Board::Loc& rhs)
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
