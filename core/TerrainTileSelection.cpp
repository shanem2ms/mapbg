#include "StdIncludes.h"
#include "TerrainTileSelection.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include "World.h"
#include "Level.h"
#include <numeric>
#include "Mesh.h"
#include "gmtl/PlaneOps.h"
#include <sstream>
#define NOMINMAX
#ifdef _WIN32
#include <Windows.h>
#endif




using namespace gmtl;

namespace sam
{

    std::atomic<size_t> TerrainTileSelection::sNumTiles = 0;
    TerrainTileSelection::TerrainTileSelection()
    {
    }

    bool TerrainTileSelection::RequestTile(const Loc& tileLoc, World* pWorld, std::shared_ptr<TerrainTile>& outTile)
    {
        Loc tl = tileLoc;
        tl.m_y = 0;
        auto itTile = m_tiles.find(tl);
        if (itTile != m_tiles.end())
        {
            if (itTile->second->IsDataReady())
            {
                outTile = itTile->second;
                return true;
            }
            else
                return false;
        }
        else
        {
            std::string val;
            if (pWorld->Level().GetTerrainChunk(tl, &val))
            {
                outTile = std::make_shared<TerrainTile>(tl, val);
                m_tiles.insert(std::make_pair(tl, outTile));
                return true;
            }
            else
                m_requestTiles.insert(tileLoc);
        }
        return false;
    }

    void TerrainTileSelection::Update(Engine& e, DrawContext& ctx)
    {

        if (m_tiles.size() > 100)
        {
            std::vector<std::pair<Loc, TerrainTile*>> alltiles;
            for (auto& pair : m_tiles)
            {
                alltiles.push_back(std::make_pair(pair.first, pair.second.get()));
            }
            std::sort(alltiles.begin(), alltiles.end(), [](const  std::pair<Loc, TerrainTile*>& l, const std::pair<Loc, TerrainTile*>& r)
                { return l.second->GetLastUsedFrame() < r.second->GetLastUsedFrame(); });
            for (auto itTile = alltiles.begin(); itTile != alltiles.begin() + 20; ++itTile)
            {
                m_tiles.erase(itTile->first);
                sNumTiles--;
            }
        }

        std::vector<std::shared_ptr<TerrainTile>> nextBuildingTiles;
        for (const Loc& oloc : m_requestTiles)
        {
            std::vector<Loc> locsParents;
            Loc p = oloc;
            p.m_y = 0;
            while (p.m_l > 0)
            {
                locsParents.push_back(p);
                p = p.Parent();
            }
            std::reverse(locsParents.begin(), locsParents.end());
            std::shared_ptr<TerrainTile> parentTile;
            for (const Loc& loc : locsParents)
            {
                auto itTile = m_tiles.find(loc);
                if (itTile == m_tiles.end())
                {
                    auto newTile = std::make_shared<TerrainTile>(loc, parentTile);
                    if (!newTile->Build(ctx.m_pWorld))
                    {
                        nextBuildingTiles.push_back(newTile);
                    }
                    itTile = m_tiles.insert(std::make_pair(loc, newTile)).first;
                    sNumTiles++;
                }
                if (!intersect(itTile->second->GetBounds(), oloc.GetBBox()))
                    break;
                parentTile = itTile->second;
            }
        }

        for (std::shared_ptr<TerrainTile>& tile : m_buildingTiles)
        {
            if (!tile->Build(ctx.m_pWorld))
                nextBuildingTiles.push_back(tile);
        }

        m_buildingTiles.swap(nextBuildingTiles);
    }

    float TerrainTileSelection::GetGroundHeight(const Point3f& pt)
    {
        int tx = (int)floor(pt[0]);
        int tz = (int)floor(pt[2]);
        Loc queryLoc(tx, 0, tz);

        static float headHeight = 0.04f;
        auto itCamTile = m_tiles.find(queryLoc);
        if (itCamTile != m_tiles.end())
        {
            return 0;// itCamTile->second->GetGroundHeight(pt) + headHeight;
        }
    }



    TerrainTileSelection::~TerrainTileSelection()
    {

    }

}
