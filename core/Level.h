#pragma once

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

        Level();
        void OpenDb(const std::string& path);

        bool GetTerrainChunk(const Loc& il, std::string* val) const;
        bool WriteTerrainChunk(const Loc& il, const char *byte, size_t len);

        bool GetOctChunk(const Loc& l, std::string* val) const;
        bool WriteOctChunk(const Loc& il, const char* byte, size_t len);
    };
}
