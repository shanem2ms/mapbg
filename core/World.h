#pragma once
#include <map>
#include <set>
#include "OctTile.h"
#include "OctTileSelection.h"

namespace leveldb
{
    class DB;
}

class SimplexNoise;
namespace sam
{

    struct DrawContext;
    class Engine;
    class Touch;
    
    class World
    {
    public:

        OctTileSelection m_octTileSelection;

        int m_width;
        int m_height;
        gmtl::Point3f m_camVel;
        float m_tiltVel;

        float m_gravityVel;

        std::shared_ptr<SceneGroup> m_worldGroup;
        std::shared_ptr<Touch> m_activeTouch;
        int m_currentTool;
        bgfx::ProgramHandle m_shader;
        leveldb::DB* m_db;

    public:
        void Layout(int w, int h);
        World();
        ~World();
        leveldb::DB* Db() {
            return m_db;
        }
        void Update(Engine& engine, DrawContext& ctx);
        void TouchDown(float x, float y, int touchId);
        void TouchDrag(float x, float y, int touchId);
        void TouchUp(int touchId);
        void KeyDown(int k);
        void KeyUp(int k);
        void OpenDb(const std::string &path);
    };

}
