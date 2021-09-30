#pragma once

#include "StdIncludes.h"
#include <map>
#include <set>
#include <functional>
#include "Loc.h"

namespace leveldb
{
    class DB;
}

namespace sam
{
    class TerrainTile;

    class Level {
        leveldb::DB* m_db;
    public:
        struct CamPos
        {
            Vec3f pos;
            Vec2f dir;
            bool flymode;
            bool inspect;
            Vec3f inspectpos;
            Vec2f inspectdir;
        };

        Level();
        void OpenDb(const std::string& path);

        bool GetTerrainChunk(const Loc& il, std::string* val) const;
        bool WriteTerrainChunk(const Loc& il, const char *byte, size_t len);

        bool GetOctChunk(const Loc& l, std::string* val) const;
        bool WriteOctChunk(const Loc& il, const char* byte, size_t len);
        bool WriteCameraPos(const CamPos &pos);
        bool GetCameraPos(CamPos& pos);
    };
}
