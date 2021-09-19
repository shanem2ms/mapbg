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

        void GetOctChunk(const Loc& il, std::function<void (const std::string &, bool)> func) const;
        bool WriteOctChunk(const Loc& il, const char* byte, size_t len);
    };
}
