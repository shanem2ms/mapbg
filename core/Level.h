#pragma once

#include <map>
#include <set>
#include <thread>
#include <condition_variable>
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
        std::thread m_loaderThread;
        std::mutex m_mtx;
        std::condition_variable m_cv;

        static void LoaderThread(void *arg);
    public:

        Level();
        void OpenDb(const std::string& path);

        bool GetTerrainChunk(const Loc& il, std::string* val) const;
        bool WriteTerrainChunk(const Loc& il, const char *byte, size_t len);

        void GetOctChunk(const Loc& il, std::function<void (const std::string &, bool)> func) const;
        bool WriteOctChunk(const Loc& il, const char* byte, size_t len);
    };
}
