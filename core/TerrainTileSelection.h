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

        std::map<Loc, std::shared_ptr<TerrainTile>> m_tiles;
        std::set<Loc> m_activeTiles;

        void Update(Engine& e, DrawContext& ctx);

        void SelectTiles(const std::vector<Loc>& tileLocs);
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
