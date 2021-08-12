#pragma once
#include <map>
#include <set>
#include "Tile.h"
#include "TileSelection.h"

class SimplexNoise;
namespace sam
{

    struct DrawContext;
    class Engine;
    class Touch;
    class Board
    {
    public:

        TileSelection m_tileSelection;

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

}
