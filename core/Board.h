#pragma once
#include <map>
#include <set>
#include "Tile.h"

class SimplexNoise;
namespace sam
{

    struct DrawContext;
    class Engine;
    class Touch;
    class Board
    {
    public:

        std::map<Loc, std::shared_ptr<Tile>> m_squares;
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
