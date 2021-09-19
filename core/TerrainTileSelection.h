#pragma once
#include <map>
#include <set>
#include "TerrainTile.h"

class SimplexNoise;
namespace sam
{

    struct DrawContext;
    class Engine;
    class Touch;
    class TerrainTileSelection
    {
    public:
        static std::atomic<size_t> sNumTiles;

        std::map<Loc, std::shared_ptr<TerrainTile>> m_tiles;
        std::set<Loc> m_activeTiles;
        std::set<Loc> m_requestTiles;

        std::vector<std::shared_ptr<TerrainTile>> m_buildingTiles;

        void Update(Engine& e, DrawContext& ctx);

        bool RequestTile(const Loc& tileLoc, World* pWorld, std::shared_ptr<TerrainTile> &outTile);
        float GetGroundHeight(const Point3f& pt);

        const std::map<Loc, std::shared_ptr<TerrainTile>>& Tiles() const
        {
            return m_tiles;
        }

    public:
        TerrainTileSelection();
        ~TerrainTileSelection();
    };
 

}
