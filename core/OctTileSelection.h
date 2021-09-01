#pragma once
#include <map>
#include <set>
#include "OctTile.h"

class SimplexNoise;
namespace sam
{

    struct DrawContext;
    class Engine;
    class Touch;
    class TerrainTileSelection;
    class OctTileSelection
    {
    public:

        std::map<Loc, std::shared_ptr<OctTile>> m_tiles;
        std::set<Loc> m_activeTiles;
        std::unique_ptr<TerrainTileSelection> m_terrainSelection;

        void Update(Engine& e, DrawContext& ctx);

        void AddTilesToGroup(std::shared_ptr<SceneGroup> grp);
        float GetGroundHeight(const Point3f& pt);

    public:
        OctTileSelection();
        ~OctTileSelection();
    };
 

}
