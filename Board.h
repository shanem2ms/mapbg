#pragma once

#include <map>
#include <set>
#include "SceneItem.h"

struct DrawContext;
class Engine;
class Touch;
class SimplexNoise;
class Board
{
public:
    struct Loc
    {
        Loc(int x, int y) :
            m_x(x),
            m_y(y),
            m_z(0),
            m_l(0) {}

        int m_x;
        int m_y;
        int m_z;
        int m_l;
    };

    struct SqPt
    {
        float height;;
        Vec2f dh;
        float sed;
    };

    static const int SquarePtsCt = 256;

    class Square : public SceneItem
    {
        float m_squareSize;
        int m_image;
        Vec2f m_vals;
        std::weak_ptr<Square> m_neighbors[9];
        Loc m_l;
        bool m_needRecalc;
        SqPt m_pts[SquarePtsCt * SquarePtsCt];
        Vec2f m_maxdh;
        Vec2f m_mindh;
        bgfx::TextureHandle m_tex;
    public:
        void Draw(DrawContext &ctx) override;
        Square(const Loc& l);
        ~Square();
        void SetSquareSize(float squareSize)
        {
            m_squareSize = squareSize;
        }

        const SqPt* Pts() const { return m_pts; }
        void SetImage(int image)
        { m_image = image; }
        virtual gmtl::AABoxf GetBounds() const;
        void SetVals(const Vec2f &v)
        {
            m_vals = v;
        }
        void SetNeighbor(int dx, int dy, std::weak_ptr<Square> sq);
        void Decomission();

    private:
        void NoiseGen();
        void ProceduralBuild(DrawContext &ctx);
        void GradientGen();
        void Erode();
    };

    std::map<Loc, std::shared_ptr<Square>> m_squares;
    std::set<Loc> m_activeSquares;

    float m_squareSize;
    int m_width;
    int m_height;
    gmtl::Point3f m_camVel;

    std::shared_ptr<SceneGroup> m_boardGroup;
    std::shared_ptr<Touch> m_activeTouch;
    int m_currentTool;
   
public:
    void Layout(int w, int h);
    Board();
    ~Board();
    void Update(Engine& engine, DrawContext &ctx);
    void TouchDown(float x, float y, int touchId);
    void TouchDrag(float x, float y, int touchId);
    void TouchUp(int touchId);
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