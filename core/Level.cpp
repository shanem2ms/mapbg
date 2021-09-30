#include "StdIncludes.h"
#include "Level.h"
#include "leveldb/dumpfile.h"
#include "leveldb/env.h"
#include "leveldb/status.h"
#include "leveldb/options.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include "leveldb/zlib_compressor.h"
#include "leveldb/decompress_allocator.h"
#include "leveldb/db.h"
#include <thread>


namespace sam
{
    class NullLogger : public leveldb::Logger {
    public:
        void Logv(const char*, va_list) override {
        }
    };


    Level::Level() :
        m_db(nullptr)
    {
    }

      
    void Level::OpenDb(const std::string& path)
    {
        //leveldb::Env* env = leveldb::Env::Default();
        leveldb::Options options;
        //create a bloom filter to quickly tell if a key is in the database or not
        options.filter_policy = leveldb::NewBloomFilterPolicy(10);

        //create a 40 mb cache (we use this on ~1gb devices)
        options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);

        //create a 4mb write buffer, to improve compression and touch the disk less
        options.write_buffer_size = 4 * 1024 * 1024;

        //disable internal logging. The default logger will still print out things to a file
        options.info_log = new NullLogger();

        //use the new raw-zip compressor to write (and read)
        options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);

        //also setup the old, slower compressor for backwards compatibility. This will only be used to read old compressed blocks.
        options.compressors[1] = new leveldb::ZlibCompressor();

        options.create_if_missing = true;

        leveldb::Status status = leveldb::DB::Open(options, path.c_str(), &m_db);
    }

    bool Level::GetTerrainChunk(const Loc& il, std::string *val) const
    {
        Loc l = il;
        l.m_l = -l.m_l;
        leveldb::Slice key((const char*)&l, sizeof(l));
        leveldb::Status status = m_db->Get(leveldb::ReadOptions(), key, val);
        return status.ok();
    }

    bool Level::WriteTerrainChunk(const Loc& il, const char* byte, size_t len)
    {
        Loc l = il;
        l.m_l = -l.m_l;
        leveldb::Slice key((const char*)&l, sizeof(l));
        leveldb::Slice val(byte, len);
        leveldb::Status status = m_db->Put(leveldb::WriteOptions(), key, val);
        return status.ok();
    }


    bool Level::GetOctChunk(const Loc& l, std::string* val) const
    {
        leveldb::Slice key((const char*)&l, sizeof(l));
        leveldb::Status status = m_db->Get(leveldb::ReadOptions(), key, val);
        return status.ok();
    }

    bool Level::WriteOctChunk(const Loc& l, const char* byte, size_t len)
    {
        leveldb::Slice key((const char*)&l, sizeof(l));
        leveldb::Slice val(byte, len);
        leveldb::Status status = m_db->Put(leveldb::WriteOptions(), key, val);
        return status.ok();
    }

    bool Level::WriteCameraPos(const Level::CamPos& pos)
    {
        leveldb::Slice key("cam", 3);
        leveldb::Slice val((const char *)&pos, sizeof(Level::CamPos));
        leveldb::Status status = m_db->Put(leveldb::WriteOptions(), key, val);
        return status.ok();
    }

    bool Level::GetCameraPos(CamPos& pos)
    {
        leveldb::Slice key("cam", 3);
        std::string val;
        leveldb::Status status = m_db->Get(leveldb::ReadOptions(), key, &val);
        if (status.ok() && val.size() == sizeof(CamPos))
        {
            memcpy(&pos, val.data(), sizeof(CamPos));
            return true;
        }
        return false;
    }
}