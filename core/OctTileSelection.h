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

        static std::atomic<size_t> sNumTiles;

        std::map<Loc, std::shared_ptr<OctTile>> m_tiles;
        std::set<Loc> m_activeTiles;
        std::unique_ptr<TerrainTileSelection> m_terrainSelection;

        void Update(Engine& e, DrawContext& ctx, const AABoxf &playerBounds);

        void AddTilesToGroup(std::shared_ptr<SceneGroup> grp);
        float GetGroundHeight(const Point3f& pt);
        void GetNearFarMidDist(float nearfarmidsq[3]);

        std::shared_ptr<OctTile> TileFromPos(const Point3f& pos);
        bool Intersects(const Point3f& pos, const Vec3f& ray, Loc& outloc, Vec3i& outpt);

    public:
        OctTileSelection();
        ~OctTileSelection();
        float m_nearfarmidsq[3];
    };
 

}
