#pragma once
#include <map>
#include <set>
#include "OctTile.h"
#include "OctTileSelection.h"
#include "TerrainTileSelection.h"
#include "Level.h"

class SimplexNoise;
namespace sam
{

    struct DrawContext;
    class Engine;
    class Touch;
    class TargetCube;
    class Physics;
    
    class World
    {
    private:

        OctTileSelection m_octTileSelection;
        TerrainTileSelection m_terrainTileSelection;

        int m_width;
        int m_height;
        gmtl::Point3f m_camVel;
        float m_tiltVel;
        bool m_flymode;
        bool m_inspectmode;

        float m_gravityVel;

        std::shared_ptr<SceneGroup> m_worldGroup;
        std::shared_ptr<Touch> m_activeTouch;
        int m_currentTool;
        bgfx::ProgramHandle m_shader;        
        std::shared_ptr<SceneItem> m_targetCube;
        std::shared_ptr<SceneItem> m_frustum;
        Level m_level;
        std::shared_ptr<Physics> m_physics;

    public:

        TerrainTileSelection &TerrainTileSelection()
        { return m_terrainTileSelection; }
        void Layout(int w, int h);
        World();
        ~World();
        Level& Level() { return m_level; }
        void Update(Engine& engine, DrawContext& ctx);
        void TouchDown(float x, float y, int touchId);
        void TouchDrag(float x, float y, int touchId);
        void TouchUp(int touchId);
        void KeyDown(int k);
        void KeyUp(int k);
        void Open(const std::string &path);
    };

}
