#include "World.h"
#include "Engine.h"
#include "HSLColor.h"
#include "SimplexNoise/SimplexNoise.h"
#include <set>

class NotImplementedException
{
};

class tarray
{
    std::vector<long long> v;
    long long m_w;
public:

    tarray(long long w, long long h) :
        v(w * h),
        m_w(w)
    {

    }

    long long &p(long long x, long long y)
    {
        return v[x * m_w + y];
    }

};

namespace Biomes {

    enum B
    {
        OCEAN = 0,

        PLAINS = 1,

        DESERT = 2,

        EXTREME_HILLS = 3,

        FOREST = 4,

        TAIGA = 5,

        SWAMPLAND = 6,

        RIVER = 7,

        HELL = 8,

        SKY = 9,

        FROZEN_OCEAN = 10,

        FROZEN_RIVER = 11,

        ICE_PLAINS = 12,

        ICE_MOUNTAINS = 13,

        MUSHROOM_ISLAND = 14,

        MUSHROOM_ISLAND_SHORE = 15,

        BEACHES = 16,

        DESERT_HILLS = 17,

        FOREST_HILLS = 18,

        TAIGA_HILLS = 19,

        SMALLER_EXTREME_HILLS = 20,

        JUNGLE = 21,

        JUNGLE_HILLS = 22,

        JUNGLE_EDGE = 23,

        DEEP_OCEAN = 24,

        STONE_BEACH = 25,

        COLD_BEACH = 26,

        BIRCH_FOREST = 27,

        BIRCH_FOREST_HILLS = 28,

        ROOFED_FOREST = 29,

        TAIGA_COLD = 30,

        TAIGA_COLD_HILLS = 31,

        REDWOOD_TAIGA = 32,

        REDWOOD_TAIGA_HILLS = 33,

        EXTREME_HILLS_WITH_TREES = 34,

        SAVANNA = 35,

        SAVANNA_ROCK = 36,

        MESA = 37,

        MESA_ROCK = 38,

        MESA_CLEAR_ROCK = 39,

    };


    static std::vector<long long> WARM_BIOMES = {
        DESERT,
        DESERT,
        DESERT,
        SAVANNA,
        SAVANNA,
        PLAINS
    };

    static std::vector<long long> MEDIUM_BIOMES = {
        FOREST,
        ROOFED_FOREST,
        EXTREME_HILLS,
        PLAINS,
        BIRCH_FOREST,
        SWAMPLAND
    };

    static std::vector<long long>  COLD_BIOMES = {
        FOREST,
        EXTREME_HILLS,
        TAIGA,
        PLAINS
    };

    static std::vector<long long>  ICE_BIOMES = {
        ICE_PLAINS,
        ICE_PLAINS,
        ICE_PLAINS,
        TAIGA_COLD
    };

    static std::map<long long, double> TEMP = {
        {
            OCEAN,
            0.5},
        {
            PLAINS,
            0.8},
        {
            DESERT,
            2.0},
        {
            EXTREME_HILLS,
            0.2},
        {
            FOREST,
            0.8},
        {
            TAIGA,
            0.2},
        {
            SWAMPLAND,
            0.8},
        {
            RIVER,
            0.5},
        {
            HELL,
            2.0},
        {
            SKY,
            0.5},
        {
            FROZEN_OCEAN,
            0.0},
        {
            FROZEN_RIVER,
            0.0},
        {
            ICE_PLAINS,
            0.0},
        {
            ICE_MOUNTAINS,
            0.0},
        {
            MUSHROOM_ISLAND,
            0.9},
        {
            MUSHROOM_ISLAND_SHORE,
            0.9},
        {
            BEACHES,
            0.8},
        {
            DESERT_HILLS,
            2.0},
        {
            FOREST_HILLS,
            0.7},
        {
            TAIGA_HILLS,
            0.25},
        {
            SMALLER_EXTREME_HILLS,
            0.2},
        {
            JUNGLE,
            0.95},
        {
            JUNGLE_HILLS,
            0.95},
        {
            JUNGLE_EDGE,
            0.95},
        {
            DEEP_OCEAN,
            0.5},
        {
            STONE_BEACH,
            0.2},
        {
            COLD_BEACH,
            0.05},
        {
            BIRCH_FOREST,
            0.6},
        {
            BIRCH_FOREST_HILLS,
            0.6},
        {
            ROOFED_FOREST,
            0.7},
        {
            TAIGA_COLD,
            -0.5},
        {
            TAIGA_COLD_HILLS,
            -0.5},
        {
            REDWOOD_TAIGA,
            0.3},
        {
            REDWOOD_TAIGA_HILLS,
            0.3},
        {
            EXTREME_HILLS_WITH_TREES,
            0.2},
        {
            SAVANNA,
            1.2},
        {
            SAVANNA_ROCK,
            1.0},
        {
            MESA,
            2.0},
        {
            MESA_ROCK,
            2.0},
        {
            MESA_CLEAR_ROCK,
            2.0} };

    std::map<long long, long long> MUTATIONS = {
        {
            PLAINS,
            PLAINS},
        {
            DESERT,
            DESERT},
        {
            EXTREME_HILLS,
            EXTREME_HILLS},
        {
            FOREST,
            FOREST},
        {
            TAIGA,
            TAIGA},
        {
            SWAMPLAND,
            SWAMPLAND},
        {
            ICE_PLAINS,
            ICE_PLAINS},
        {
            JUNGLE,
            JUNGLE},
        {
            JUNGLE_EDGE,
            JUNGLE_EDGE},
        {
            BIRCH_FOREST,
            BIRCH_FOREST},
        {
            BIRCH_FOREST_HILLS,
            BIRCH_FOREST_HILLS},
        {
            ROOFED_FOREST,
            ROOFED_FOREST},
        {
            TAIGA_COLD,
            TAIGA_COLD},
        {
            REDWOOD_TAIGA,
            REDWOOD_TAIGA},
        {
            REDWOOD_TAIGA_HILLS,
            REDWOOD_TAIGA_HILLS},
        {
            EXTREME_HILLS_WITH_TREES,
            EXTREME_HILLS_WITH_TREES},
        {
            SAVANNA,
            SAVANNA},
        {
            SAVANNA_ROCK,
            SAVANNA_ROCK},
        {
            MESA,
            MESA},
        {
            MESA_ROCK,
            MESA_ROCK},
        {
            MESA_CLEAR_ROCK,
            MESA_CLEAR_ROCK} };

    std::map<long long, long long> CLASSES = {
        {
            OCEAN,
            OCEAN},
        {
            PLAINS,
            PLAINS},
        {
            DESERT,
            DESERT},
        {
            EXTREME_HILLS,
            EXTREME_HILLS},
        {
            FOREST,
            FOREST},
        {
            TAIGA,
            TAIGA},
        {
            SWAMPLAND,
            SWAMPLAND},
        {
            RIVER,
            RIVER},
        {
            HELL,
            HELL},
        {
            SKY,
            SKY},
        {
            FROZEN_OCEAN,
            OCEAN},
        {
            FROZEN_RIVER,
            RIVER},
        {
            ICE_PLAINS,
            ICE_PLAINS},
        {
            ICE_MOUNTAINS,
            ICE_PLAINS},
        {
            MUSHROOM_ISLAND,
            MUSHROOM_ISLAND},
        {
            MUSHROOM_ISLAND_SHORE,
            MUSHROOM_ISLAND},
        {
            BEACHES,
            BEACHES},
        {
            DESERT_HILLS,
            DESERT},
        {
            FOREST_HILLS,
            FOREST},
        {
            TAIGA_HILLS,
            TAIGA},
        {
            SMALLER_EXTREME_HILLS,
            EXTREME_HILLS},
        {
            JUNGLE,
            JUNGLE},
        {
            JUNGLE_HILLS,
            JUNGLE},
        {
            JUNGLE_EDGE,
            JUNGLE},
        {
            DEEP_OCEAN,
            OCEAN},
        {
            STONE_BEACH,
            STONE_BEACH},
        {
            COLD_BEACH,
            BEACHES},
        {
            BIRCH_FOREST,
            FOREST},
        {
            BIRCH_FOREST_HILLS,
            FOREST},
        {
            ROOFED_FOREST,
            FOREST},
        {
            TAIGA_COLD,
            TAIGA},
        {
            TAIGA_COLD_HILLS,
            TAIGA},
        {
            REDWOOD_TAIGA,
            TAIGA},
        {
            REDWOOD_TAIGA_HILLS,
            TAIGA},
        {
            EXTREME_HILLS_WITH_TREES,
            EXTREME_HILLS},
        {
            SAVANNA,
            SAVANNA},
        {
            SAVANNA_ROCK,
            SAVANNA},
        {
            MESA,
            MESA},
        {
            MESA_ROCK,
            MESA},
        {
            MESA_CLEAR_ROCK,
            MESA} };

    static long long TEMP_COLD = 3;

    static long long TEMP_MEDIUM = 2;

    static long long TEMP_WARM = 1;

    static long long get_temp_category(long long biome) {
        if (TEMP.find(biome) == TEMP.end()) {
            return TEMP_MEDIUM;
        }
        if (TEMP[biome] < 0.2) {
            return TEMP_COLD;
        }
        else if (TEMP[biome] < 1.0) {
            return TEMP_MEDIUM;
        }
        return TEMP_WARM;
    }

    static bool is_snowy(long long biome) {
        static std::set<long long> h = {
        {
            FROZEN_OCEAN}, {
            FROZEN_RIVER}, {
            ICE_PLAINS}, {
            ICE_MOUNTAINS}, {
            COLD_BEACH}, {
            TAIGA_COLD}, {
            TAIGA_COLD_HILLS} };
        return h.find(biome) != h.end();
    }

    static long long get_biome_class(long long biome) {
        auto itCl = CLASSES.find(biome);
        if (itCl == CLASSES.end())
            return -1;
        return itCl->second;
    }

    // Don't support mutations yet
    static bool is_mutation(long long biome) {
        return false;
    }

    static long long get_mutation_for_biome(long long biome) {
        auto itCl = MUTATIONS.find(biome);
        if (itCl == MUTATIONS.end())
            return -1;
        return itCl->second;
    }
}


class Layers {
public:
    static long long convolute(long long x, long long a) {
        return x * (x * 6364136223846793005L + 1442695040888963407L) + a;
    }

    class GenLayer {
    public:
        long long base_seed;
        long long world_gen_seed;
        GenLayer* parent = nullptr;
        long long chunk_seed;
        GenLayer(long long baseSeed) {
            this->base_seed = baseSeed;
            this->base_seed = convolute(this->base_seed, base_seed);
            this->base_seed = convolute(this->base_seed, base_seed);
            this->base_seed = convolute(this->base_seed, base_seed);
            this->world_gen_seed = 0;
            this->parent = nullptr;
            this->chunk_seed = 0;
        };

        virtual void init_world_gen_seed(long long seed) {
            this->world_gen_seed = seed;
            if (this->parent != nullptr) {
                this->parent->init_world_gen_seed(seed);
            }
            this->world_gen_seed = convolute(this->world_gen_seed, this->base_seed);
            this->world_gen_seed = convolute(this->world_gen_seed, this->base_seed);
            this->world_gen_seed = convolute(this->world_gen_seed, this->base_seed);
        }

        virtual void init_chunk_seed(long long x, long long z) {
            this->chunk_seed = this->world_gen_seed;
            this->chunk_seed = convolute(this->chunk_seed, x);
            this->chunk_seed = convolute(this->chunk_seed, z);
            this->chunk_seed = convolute(this->chunk_seed, x);
            this->chunk_seed = convolute(this->chunk_seed, z);
        }

        virtual long long next_int(long long r) {
            auto i = (this->chunk_seed >> 24) % r;
            if (i < 0) {
                i += r;
            }
            this->chunk_seed = convolute(this->chunk_seed, this->world_gen_seed);
            return i;
        }

        virtual long long select_random(const std::vector<long long> &choices) {
            return choices[this->next_int(choices.size())];
        }

        virtual bool biomes_equal_or_mesa_plateau(long long a, long long b) {
            if (a == b) {
                return true;
            }
            if (a == Biomes::MESA_ROCK || a == Biomes::MESA_CLEAR_ROCK) {
                return b == Biomes::MESA_ROCK || b == Biomes::MESA_CLEAR_ROCK;
            }
            return Biomes::get_biome_class(a) == Biomes::get_biome_class(b);
        }

        virtual bool is_oceanic(long long a) {
            return a == Biomes::OCEAN || a == Biomes::DEEP_OCEAN || a == Biomes::FROZEN_OCEAN;
        }

        virtual tarray get_ints(long long x, long long y, long long width, long long height) {
            throw new NotImplementedException();
        }
    };

    class Island
        : public GenLayer {
    public:
        Island(long long base_seed) : GenLayer(base_seed)
        {
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            for (int i = 0; i < width; ++i) {
                for (int j = 0; j < height; ++j) {
                    this->init_chunk_seed(x + i, y + j);
                    new_arr.p(i, j) = this->next_int(10) == 0 ? 1 : 0;
                }
            }
            if ((-width < x) && (x <= 0) && (-height < y) && (y <= 0)) {
                new_arr.p(-x, -y) = 1;
            }
            return new_arr;
        }
    };

    class Zoom
        : public GenLayer {

    public:
        Zoom(long long base_seed, GenLayer* _parent) :
            GenLayer(base_seed) {
            this->parent = _parent;
        }

        virtual long long mode_or_random(long long a, long long b, long long c, long long d) {
            if (b == c && c == d) {
                return b;
            }
            else if (a == b && a == c) {
                return a;
            }
            else if (a == b && a == d) {
                return a;
            }
            else if (a == c && a == d) {
                return a;
            }
            else if (a == b && c != d) {
                return a;
            }
            else if (a == c && b != d) {
                return a;
            }
            else if (a == d && b != c) {
                return a;
            }
            else if (b == c && a != d) {
                return b;
            }
            else if (b == d && a != c) {
                return b;
            }
            return c == d && a != b ? c : this->select_random(
                { a, b, c, d });
        }
        tarray get_ints(long long x, long long y, long long width, long long height) override {
            auto px = x >> 1;
            auto py = y >> 1;
            auto pwidth = (width >> 1) + 2;
            auto pheight = (height >> 1) + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            auto iwidth = (pwidth - 1) << 1;
            auto iheight = (pheight - 1) << 1;
            tarray i_arr(iwidth, iheight);
            for (int j = 0; j < pheight - 1; ++j)
            {
                for (int i = 0; i < pwidth - 1; ++i) {
                    this->init_chunk_seed((px + i) * 2, (py + j) * 2);
                    i_arr.p(2 * i, 2 * j) = p_arr.p(i, j);
                    i_arr.p(2 * i, 2 * j + 1) = this->select_random({ p_arr.p(i, j), p_arr.p(i, j + 1) });
                    i_arr.p(2 * i + 1, 2 * j) = this->select_random({ p_arr.p(i, j), p_arr.p(i + 1, j) });
                    i_arr.p(2 * i + 1, 2 * j + 1) = this->mode_or_random(p_arr.p(i, j), p_arr.p(i + 1, j), p_arr.p(i, j + 1), p_arr.p(i + 1, j + 1));
                }
            }
            auto startx = x & 1;
            auto starty = y & 1;
            tarray new_arr(width, height);
            for (int j = 0; j < height; ++j)
            {
                for (int i = 0; i < width; ++i)
                {
                    new_arr.p(i, j) = i_arr.p(i + startx, j + starty);
                }
            }
            return new_arr;
        }
    };

    class FuzzyZoom : public Zoom {
    public:
        FuzzyZoom(long long seed, GenLayer* p) : Zoom(seed, p)
        { }
        long long mode_or_random(long long a, long long b, long long c, long long d) override {
            return this->select_random({ a, b, c, d });
        }
    };

    class AddIsland
        : public GenLayer {
    public:
        AddIsland(long long base_seed, GenLayer* _parent)
            : GenLayer(base_seed) {
            this->parent = _parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int i = 0; i < width; ++i) {
                for (int j = 0; j < height; ++j) {
                    this->init_chunk_seed(x + i, y + j);
                    auto a = p_arr.p(i, j);
                    auto b = p_arr.p(i + 2, j);
                    auto c = p_arr.p(i, j + 2);
                    auto d = p_arr.p(i + 2, j + 2);
                    auto k = p_arr.p(i + 1, j + 1);
                    if (k != 0 || a == 0 && b == 0 && c == 0 && d == 0) {
                        if (k > 0 && (a == 0 || b == 0 || c == 0 || d == 0)) {
                            if (this->next_int(5) == 0) {
                                if (k == 4) {
                                    new_arr.p(i, j) = 4;
                                }
                                else {
                                    new_arr.p(i, j) = 0;
                                }
                            }
                            else {
                                new_arr.p(i, j) = k;
                            }
                        }
                        else {
                            new_arr.p(i, j) = k;
                        }
                    }
                    else {
                        long long l2 = 1;
                        long long n = 1;
                        if (a != 0) {
                            if (this->next_int(l2) == 0) {
                                n = a;
                            }
                            l2 += 1;
                        }
                        if (b != 0) {
                            if (this->next_int(l2) == 0) {
                                n = b;
                            }
                            l2 += 1;
                        }
                        if (c != 0) {
                            if (this->next_int(l2) == 0) {
                                n = c;
                            }
                            l2 += 1;
                        }
                        if (d != 0) {
                            if (this->next_int(l2) == 0) {
                                n = d;
                            }
                            l2 += 1;
                        }
                        if (this->next_int(3) == 0) {
                            new_arr.p(i, j) = n;
                        }
                        else if (n == 4) {
                            new_arr.p(i, j) = 4;
                        }
                        else {
                            new_arr.p(i, j) = 0;
                        }
                    }
                }
            }
            return new_arr;
        }
    };

    class RemoveTooMuchOcean
        : public GenLayer {
    public:
        RemoveTooMuchOcean(long long base_seed, GenLayer* parent) :GenLayer(base_seed) {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    auto a = p_arr.p(i + 1, j);
                    auto b = p_arr.p(i + 2, j + 1);
                    auto c = p_arr.p(i, j + 1);
                    auto d = p_arr.p(i + 1, j + 2);
                    auto k = p_arr.p(i + 1, j + 1);
                    new_arr.p(i, j) = k;
                    if (k == 0 && a == 0 && b == 0 && c == 0 && d == 0 && this->next_int(2) == 0) {
                        new_arr.p(i, j) = 1;
                    }
                }
            }
            return new_arr;
        }
    };

    class AddSnow
        : public GenLayer {
    public:
        AddSnow(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto p_arr = this->parent->get_ints(x, y, width, height);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    auto k = p_arr.p(i, j);
                    if (k == 0) {
                        new_arr.p(i, j) = 0;
                    }
                    else {
                        auto r = this->next_int(6);
                        if (r == 0) {
                            r = 4;
                        }
                        else if (r <= 1) {
                            r = 3;
                        }
                        else {
                            r = 1;
                        }
                        new_arr.p(i, j) = r;
                    }
                }
            }
            return new_arr;
        }
    };

    class EdgeCoolWarm
        : public GenLayer {
    public:
        EdgeCoolWarm(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    auto k = p_arr.p(i + 1, j + 1);
                    if (k == 1) {
                        auto a = p_arr.p(i + 1, j);
                        auto b = p_arr.p(i + 2, j + 1);
                        auto c = p_arr.p(i, j + 1);
                        auto d = p_arr.p(i + 1, j + 2);
                        auto flag1 = a == 3 || b == 3 || c == 3 || d == 3;
                        auto flag2 = a == 4 || b == 4 || c == 4 || d == 4;
                        if (flag1 || flag2) {
                            k = 2;
                        }
                    }
                    new_arr.p(i, j) = k;
                }
            }
            return new_arr;
        }
    };

    class EdgeHeatIce
        : public GenLayer {
    public:
        EdgeHeatIce(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    auto k = p_arr.p(i + 1, j + 1);
                    if (k == 4) {
                        auto a = p_arr.p(i + 1, j);
                        auto b = p_arr.p(i + 2, j + 1);
                        auto c = p_arr.p(i, j + 1);
                        auto d = p_arr.p(i + 1, j + 2);
                        auto flag1 = a == 2 || b == 2 || c == 2 || d == 2;
                        auto flag2 = a == 1 || b == 1 || c == 1 || d == 1;
                        if (flag1 || flag2) {
                            k = 3;
                        }
                    }
                    new_arr.p(i, j) = k;
                }
            }
            return new_arr;
        }
    };

    class EdgeSpecial
        : public GenLayer {
    public:
        EdgeSpecial(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto p_arr = this->parent->get_ints(x, y, width, height);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    auto k = p_arr.p(i, j);
                    if (k != 0 && this->next_int(13) == 0) {
                        k = (int32_t)(k) | 1 + this->next_int(15) << 8 & 3840;
                    }
                    new_arr.p(i, j) = k;
                }
            }
            return new_arr;
        }
    };

    class AddMushroomIsland
        : public GenLayer {
    public:
        AddMushroomIsland(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    auto a = p_arr.p(i, j);
                    auto b = p_arr.p(i + 2, j);
                    auto c = p_arr.p(i, j + 2);
                    auto d = p_arr.p(i + 2, j + 2);
                    auto k = p_arr.p(i + 1, j + 1);
                    if (k == 0 && a == 0 && b == 0 && c == 0 && d == 0 && this->next_int(100) == 0) {
                        new_arr.p(i, j) = Biomes::MUSHROOM_ISLAND;
                    }
                    else {
                        new_arr.p(i, j) = k;
                    }
                }
            }
            return new_arr;
        }
    };

    class DeepOcean
        : public GenLayer {
    public:
        DeepOcean(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    auto a = p_arr.p(i + 1, j);
                    auto b = p_arr.p(i + 2, j + 1);
                    auto c = p_arr.p(i, j + 1);
                    auto d = p_arr.p(i + 1, j + 2);
                    auto k = p_arr.p(i + 1, j + 1);
                    if (k == 0 && a == 0 && b == 0 && c == 0 && d == 0) {
                        new_arr.p(i, j) = Biomes::DEEP_OCEAN;
                    }
                    else {
                        new_arr.p(i, j) = k;
                    }
                }
            }
            return new_arr;
        }
    };

    class Biome
        : public GenLayer {
    public:
        Biome(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto p_arr = this->parent->get_ints(x, y, width, height);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    auto k = p_arr.p(i, j);
                    auto r = ((int32_t)(k) & 3840) >> 8;
                    k = (int32_t)(k) & -3841;
                    if (k == 0 || k == Biomes::DEEP_OCEAN || k == Biomes::FROZEN_OCEAN || k == Biomes::MUSHROOM_ISLAND) {
                        new_arr.p(i, j) = k;
                    }
                    else if (k == 1) {
                        if (r > 0) {
                            if (this->next_int(3) == 0) {
                                new_arr.p(i, j) = Biomes::MESA_CLEAR_ROCK;
                            }
                            else {
                                new_arr.p(i, j) = Biomes::MESA_ROCK;
                            }
                        }
                        else {
                            new_arr.p(i, j) = this->select_random(Biomes::WARM_BIOMES);
                        }
                    }
                    else if (k == 2) {
                        if (r > 0) {
                            new_arr.p(i, j) = Biomes::JUNGLE;
                        }
                        else {
                            new_arr.p(i, j) = this->select_random({ Biomes::MEDIUM_BIOMES });
                        }
                    }
                    else if (k == 3) {
                        if (r > 0) {
                            new_arr.p(i, j) = Biomes::REDWOOD_TAIGA;
                        }
                        else {
                            new_arr.p(i, j) = this->select_random({ Biomes::COLD_BIOMES });
                        }
                    }
                    else if (k == 4) {
                        new_arr.p(i, j) = this->select_random({ Biomes::ICE_BIOMES });
                    }
                    else {
                        new_arr.p(i, j) = Biomes::MUSHROOM_ISLAND;
                    }
                }
            }
            return new_arr;
        }
    };

    class BiomeEdge
        : public GenLayer {
    public:
        BiomeEdge(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        virtual bool can_be_neighbors(long long a, long long b) {
            if (this->biomes_equal_or_mesa_plateau(a, b)) {
                return true;
            }
            auto temp_a = Biomes::get_temp_category(a);
            auto temp_b = Biomes::get_temp_category(b);
            if (temp_a == temp_b || temp_a == Biomes::TEMP_MEDIUM || temp_b == Biomes::TEMP_MEDIUM) {
                return true;
            }
            return false;
        }

        virtual bool replace_biome_edge(
            tarray &p_arr,
            tarray &new_arr,
            long long i,
            long long j,
            long long k,
            long long to_replace,
            long long replace_with) {
            if (k != to_replace) {
                return false;
            }
            else {
                auto a = p_arr.p(i + 1, j);
                auto b = p_arr.p(i + 2, j + 1);
                auto c = p_arr.p(i, j + 1);
                auto d = p_arr.p(i + 1, j + 2);
                if (this->biomes_equal_or_mesa_plateau(a, k) && this->biomes_equal_or_mesa_plateau(b, k) && this->biomes_equal_or_mesa_plateau(c, k) && this->biomes_equal_or_mesa_plateau(d, k)) {
                    new_arr.p(i, j) = (int32_t)(to_replace);
                }
                else {
                    new_arr.p(i, j) = (int32_t)(replace_with);
                }
                return true;
            }
        }

        virtual bool replace_biome_edge_if_necessary(
            tarray &p_arr,
            tarray &new_arr,
            long long i,
            long long j,
            long long k,
            long long to_replace,
            long long replace_with) {
            if (!this->biomes_equal_or_mesa_plateau(k, to_replace)) {
                return false;
            }
            else {
                auto a = p_arr.p(i + 1, j);
                auto b = p_arr.p(i + 2, j + 1);
                auto c = p_arr.p(i, j + 1);
                auto d = p_arr.p(i + 1, j + 2);
                if (this->can_be_neighbors(a, k) && this->can_be_neighbors(b, k) && this->can_be_neighbors(c, k) && this->can_be_neighbors(d, k)) {
                    new_arr.p(i, j) = to_replace;
                }
                else {
                    new_arr.p(i, j) = replace_with;
                }
                return true;
            }
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    auto k = p_arr.p(i + 1, j + 1);
                    if (!this->replace_biome_edge_if_necessary(p_arr, new_arr, i, j, k, Biomes::EXTREME_HILLS, Biomes::SMALLER_EXTREME_HILLS) && !this->replace_biome_edge(p_arr, new_arr, i, j, k, Biomes::MESA_ROCK, Biomes::MESA) && !this->replace_biome_edge(p_arr, new_arr, i, j, k, Biomes::MESA_CLEAR_ROCK, Biomes::MESA) && !this->replace_biome_edge(p_arr, new_arr, i, j, k, Biomes::REDWOOD_TAIGA, Biomes::TAIGA)) {
                        auto a = p_arr.p(i + 1, j);
                        auto b = p_arr.p(i + 2, j + 1);
                        auto c = p_arr.p(i, j + 1);
                        auto d = p_arr.p(i + 1, j + 2);
                        if (k == Biomes::DESERT) {
                            if (a != Biomes::ICE_PLAINS && b != Biomes::ICE_PLAINS && c != Biomes::ICE_PLAINS && d != Biomes::ICE_PLAINS) {
                                new_arr.p(i, j) = k;
                            }
                            else {
                                new_arr.p(i, j) = Biomes::EXTREME_HILLS_WITH_TREES;
                            }
                        }
                        else if (k == Biomes::SWAMPLAND) {
                            if (a != Biomes::DESERT && b != Biomes::DESERT && c != Biomes::DESERT && d != Biomes::DESERT && a != Biomes::TAIGA_COLD && b != Biomes::TAIGA_COLD && c != Biomes::TAIGA_COLD && d != Biomes::TAIGA_COLD) {
                                if (a != Biomes::JUNGLE && b != Biomes::JUNGLE && c != Biomes::JUNGLE && d != Biomes::JUNGLE) {
                                    new_arr.p(i, j) = k;
                                }
                                else {
                                    new_arr.p(i, j) = Biomes::JUNGLE_EDGE;
                                }
                            }
                            else {
                                new_arr.p(i, j) = Biomes::PLAINS;
                            }
                        }
                        else {
                            new_arr.p(i, j) = k;
                        }
                    }
                }
            }
            return new_arr;
        }
    };

    class RiverInit
        : public GenLayer {
    public:
        RiverInit(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto p_arr = this->parent->get_ints(x, y, width, height);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    if (p_arr.p(i, j) > 0) {
                        new_arr.p(i, j) = this->next_int(299999) + 2;
                    }
                    else {
                        new_arr.p(i, j) = 0;
                    }
                }
            }
            return new_arr;
        }
    };

    class River
        : public GenLayer {
    public:
        River(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        virtual long long river_filter(long long k) {
            if (k >= 2) {
                return 2 + (k & 1);
            }
            else {
                return k;
            }
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    auto a = this->river_filter(p_arr.p(i + 1, j));
                    auto b = this->river_filter(p_arr.p(i + 2, j + 1));
                    auto c = this->river_filter(p_arr.p(i, j + 1));
                    auto d = this->river_filter(p_arr.p(i + 1, j + 2));
                    auto k = this->river_filter(p_arr.p(i + 1, j + 1));
                    if (k == a && k == b && k == c && k == d) {
                        new_arr.p(i, j) = -1;
                    }
                    else {
                        new_arr.p(i, j) = Biomes::RIVER;
                    }
                }
            }
            return new_arr;
        }
    };

    class Hills
        : public GenLayer {
    public:
        GenLayer *river_layer;

        Hills(long long base_seed, GenLayer* parent, GenLayer *river_layer) : GenLayer(base_seed) {
            this->parent = parent;
            this->river_layer = river_layer;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            long long c;
            long long mutated;
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            auto r_arr = this->river_layer->get_ints(x, y, width, height);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    auto k = p_arr.p(i + 1, j + 1);
                    auto l = r_arr.p(i, j);
                    auto mutate_hills = l >= 2 && (l - 2) % 29 == 0;
                    if (k != 0 && l >= 2 && (l - 2) % 29 == 1 && !Biomes::is_mutation(k)) {
                        // Mutate biome
                        mutated = Biomes::get_mutation_for_biome(k);
                        new_arr.p(i, j) = mutated >=0 ? mutated : k;
                    }
                    else if (this->next_int(3) != 0 && !mutate_hills) {
                        new_arr.p(i, j) = k;
                    }
                    else {
                        auto n = k;
                        if (k == Biomes::DESERT) {
                            n = Biomes::DESERT_HILLS;
                        }
                        else if (k == Biomes::FOREST) {
                            n = Biomes::FOREST_HILLS;
                        }
                        else if (k == Biomes::BIRCH_FOREST) {
                            n = Biomes::BIRCH_FOREST_HILLS;
                        }
                        else if (k == Biomes::ROOFED_FOREST) {
                            n = Biomes::PLAINS;
                        }
                        else if (k == Biomes::TAIGA) {
                            n = Biomes::TAIGA_HILLS;
                        }
                        else if (k == Biomes::REDWOOD_TAIGA) {
                            n = Biomes::REDWOOD_TAIGA_HILLS;
                        }
                        else if (k == Biomes::TAIGA_COLD) {
                            n = Biomes::TAIGA_COLD_HILLS;
                        }
                        else if (k == Biomes::PLAINS) {
                            if (this->next_int(3) == 0) {
                                n = Biomes::FOREST_HILLS;
                            }
                            else {
                                n = Biomes::FOREST;
                            }
                        }
                        else if (k == Biomes::ICE_PLAINS) {
                            n = Biomes::ICE_MOUNTAINS;
                        }
                        else if (k == Biomes::JUNGLE) {
                            n = Biomes::JUNGLE_HILLS;
                        }
                        else if (k == Biomes::OCEAN) {
                            n = Biomes::DEEP_OCEAN;
                        }
                        else if (k == Biomes::EXTREME_HILLS) {
                            n = Biomes::EXTREME_HILLS_WITH_TREES;
                        }
                        else if (k == Biomes::SAVANNA) {
                            n = Biomes::SAVANNA_ROCK;
                        }
                        else if (this->biomes_equal_or_mesa_plateau(k, Biomes::MESA_ROCK)) {
                            n = Biomes::MESA;
                        }
                        else if (k == Biomes::DEEP_OCEAN && this->next_int(3) == 0) {
                            c = this->next_int(2);
                            if (c == 0) {
                                n = Biomes::PLAINS;
                            }
                            else {
                                n = Biomes::FOREST;
                            }
                        }
                        if (mutate_hills && n != k) {
                            mutated = Biomes::get_mutation_for_biome(n);
                            n = mutated >= 0 ? mutated : k;
                        }
                        if (n == k) {
                            new_arr.p(i, j) = k;
                        }
                        else {
                            auto a = p_arr.p(i + 1, j);
                            auto b = p_arr.p(i + 2, j + 1);
                            c = p_arr.p(i, j + 1);
                            auto d = p_arr.p(i + 1, j + 2);
                            auto num_same_neighbors = 0;
                            if (this->biomes_equal_or_mesa_plateau(a, k)) {
                                num_same_neighbors += 1;
                            }
                            if (this->biomes_equal_or_mesa_plateau(b, k)) {
                                num_same_neighbors += 1;
                            }
                            if (this->biomes_equal_or_mesa_plateau(c, k)) {
                                num_same_neighbors += 1;
                            }
                            if (this->biomes_equal_or_mesa_plateau(d, k)) {
                                num_same_neighbors += 1;
                            }
                            if (num_same_neighbors >= 3) {
                                new_arr.p(i, j) = n;
                            }
                            else {
                                new_arr.p(i, j) = k;
                            }
                        }
                    }
                }
            }
            return new_arr;
        }
    };

    class Smooth
        : public GenLayer {
    public:
        Smooth(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    auto a = p_arr.p(i, j + 1);
                    auto b = p_arr.p(i + 2, j + 1);
                    auto c = p_arr.p(i + 1, j);
                    auto d = p_arr.p(i + 1, j + 2);
                    auto k = p_arr.p(i + 1, j + 1);
                    if (a == b && c == d) {
                        this->init_chunk_seed(x + i, y + j);
                        if (this->next_int(2) == 0) {
                            k = a;
                        }
                        else {
                            k = c;
                        }
                    }
                    else {
                        if (a == b) {
                            k = a;
                        }
                        if (c == d) {
                            k = c;
                        }
                    }
                    new_arr.p(i, j) = k;
                }
            }
            return new_arr;
        }
    };

    class RiverMix
        : public GenLayer {
    public:
        GenLayer *river_layer;
        RiverMix(long long base_seed, GenLayer* parent, GenLayer *river_layer) : GenLayer(base_seed)
        {
            this->parent = parent;
            this->river_layer = river_layer;
        }

        void init_world_gen_seed(long long seed) override {
            GenLayer::init_world_gen_seed(seed);
            if (this->river_layer != nullptr) {
                this->river_layer->init_world_gen_seed(seed);
            }
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto p_arr = this->parent->get_ints(x, y, width, height);
            auto r_arr = this->river_layer->get_ints(x, y, width, height);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    if (p_arr.p(i, j) != Biomes::OCEAN && p_arr.p(i, j) != Biomes::DEEP_OCEAN) {
                        if (r_arr.p(i, j) == Biomes::RIVER) {
                            if (p_arr.p(i, j) == Biomes::ICE_PLAINS) {
                                new_arr.p(i, j) = Biomes::FROZEN_RIVER;
                            }
                            else if (p_arr.p(i, j) != Biomes::MUSHROOM_ISLAND && p_arr.p(i, j) != Biomes::MUSHROOM_ISLAND_SHORE) {
                                new_arr.p(i, j) = (int32_t)(r_arr.p(i, j)) & 255;
                            }
                            else {
                                new_arr.p(i, j) = Biomes::MUSHROOM_ISLAND_SHORE;
                            }
                        }
                        else {
                            new_arr.p(i, j) = p_arr.p(i, j);
                        }
                    }
                    else {
                        new_arr.p(i, j) = p_arr.p(i, j);
                    }
                }
            }
            return new_arr;
        }
    };

    class Shore
        : public GenLayer {
    public:
        Shore(long long base_seed, GenLayer* parent) : GenLayer(base_seed)
        {
            this->parent = parent;
        }

        virtual bool is_mesa(long long k) {
            return k == Biomes::MESA || k == Biomes::MESA_ROCK || k == Biomes::MESA_CLEAR_ROCK;
        }

        virtual bool is_jungle_compatible(long long k) {
            return k == Biomes::JUNGLE_EDGE || k == Biomes::JUNGLE || k == Biomes::JUNGLE_HILLS || k == Biomes::FOREST || k == Biomes::TAIGA || this->is_oceanic(k);
        }

        tarray get_ints(long long x, long long y, long long width, long long height) override {
            tarray new_arr(width, height);
            auto px = x - 1;
            auto py = y - 1;
            auto pwidth = width + 2;
            auto pheight = height + 2;
            auto p_arr = this->parent->get_ints(px, py, pwidth, pheight);
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    this->init_chunk_seed(x + i, y + j);
                    auto a = p_arr.p(i, j + 1);
                    auto b = p_arr.p(i + 2, j + 1);
                    auto c = p_arr.p(i + 1, j);
                    auto d = p_arr.p(i + 1, j + 2);
                    auto k = p_arr.p(i + 1, j + 1);
                    if (k == Biomes::MUSHROOM_ISLAND) {
                        if (a != Biomes::OCEAN && b != Biomes::OCEAN && c != Biomes::OCEAN && d != Biomes::OCEAN) {
                            new_arr.p(i, j) = k;
                        }
                        else {
                            new_arr.p(i, j) = Biomes::MUSHROOM_ISLAND_SHORE;
                        }
                    }
                    else if (k == Biomes::JUNGLE || k == Biomes::JUNGLE_HILLS || k == Biomes::JUNGLE_EDGE) {
                        if (this->is_jungle_compatible(a) && this->is_jungle_compatible(b) && this->is_jungle_compatible(c) && this->is_jungle_compatible(d)) {
                            if (this->is_oceanic(a) || this->is_oceanic(b) || this->is_oceanic(c) || this->is_oceanic(d)) {
                                new_arr.p(i, j) = Biomes::BEACHES;
                            }
                            else {
                                new_arr.p(i, j) = k;
                            }
                        }
                        else {
                            new_arr.p(i, j) = Biomes::JUNGLE_EDGE;
                        }
                    }
                    else if (k == Biomes::EXTREME_HILLS || k == Biomes::EXTREME_HILLS_WITH_TREES || k == Biomes::SMALLER_EXTREME_HILLS) {
                        if (this->is_oceanic(a) || this->is_oceanic(b) || this->is_oceanic(c) || this->is_oceanic(d)) {
                            new_arr.p(i, j) = Biomes::STONE_BEACH;
                        }
                        else {
                            new_arr.p(i, j) = k;
                        }
                    }
                    else if (Biomes::is_snowy(k)) {
                        if (!this->is_oceanic(k) && (this->is_oceanic(a) || this->is_oceanic(b) || this->is_oceanic(c) || this->is_oceanic(d))) {
                            new_arr.p(i, j) = Biomes::COLD_BEACH;
                        }
                        else {
                            new_arr.p(i, j) = k;
                        }
                    }
                    else if (k == Biomes::MESA || k == Biomes::MESA_ROCK) {
                        if (!this->is_oceanic(a) && !this->is_oceanic(b) && !this->is_oceanic(c) && !this->is_oceanic(d) && (!this->is_mesa(a) || !this->is_mesa(b) || !this->is_mesa(c) || !this->is_mesa(d))) {
                            new_arr.p(i, j) = Biomes::DESERT;
                        }
                        else {
                            new_arr.p(i, j) = k;
                        }
                    }
                    else if (k != Biomes::OCEAN && k != Biomes::DEEP_OCEAN && k != Biomes::RIVER && k != Biomes::SWAMPLAND) {
                        if (this->is_oceanic(a) || this->is_oceanic(b) || this->is_oceanic(c) || this->is_oceanic(d)) {
                            new_arr.p(i, j) = Biomes::BEACHES;
                        }
                        else {
                            new_arr.p(i, j) = k;
                        }
                    }
                    else {
                        new_arr.p(i, j) = k;
                    }
                }
            }
            return new_arr;
        }
    };
};


namespace Vis
{
    struct RGB
    {
        RGB(unsigned char r, unsigned char g, unsigned char b)
        {
            c[0] = r;
            c[1] = g;
            c[2] = b;
        }
        unsigned char c[3];
    };
    static long NUM_BIOMES = 64;

    static RGB hyperbiome_colors[] = {
        RGB(
            21,
            128,
            209
        ),
        RGB(
            227,
            217,
            159
        ),
        RGB(
            163,
            207,
            114
        ),
        RGB(
            45,
            102,
            43
        ),
        RGB(
            209,
            240,
            238
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            224,
            171,
            245
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            87,
            207,
            54
        ),
        RGB(
            21,
            109,
            209
        )
    };

    static RGB biome_colors[] = {
        RGB(
            21,
            128,
            209
        ),
        RGB(
            129,
            194,
            101
        ),
        RGB(
            227,
            217,
            159
        ),
        RGB(
            184,
            194,
            178
        ),
        RGB(
            55,
            125,
            52
        ),
        RGB(
            45,
            102,
            43
        ),
        RGB(
            78,
            125,
            25
        ),
        RGB(
            23,
            173,
            232
        ),
        RGB(
            148,
            41,
            15
        ),
        RGB(
            245,
            245,
            245
        ),
        RGB(
            181,
            217,
            245
        ),
        RGB(
            181,
            217,
            245
        ),
        RGB(
            209,
            240,
            238
        ),
        RGB(
            235,
            245,
            244
        ),
        RGB(
            224,
            171,
            245
        ),
        RGB(
            213,
            170,
            230
        ),
        RGB(
            227,
            217,
            159
        ),
        RGB(
            227,
            217,
            159
        ),
        RGB(
            71,
            145,
            68
        ),
        RGB(
            45,
            102,
            43
        ),
        RGB(
            169,
            199,
            151
        ),
        RGB(
            56,
            186,
            0
        ),
        RGB(
            86,
            204,
            35
        ),
        RGB(
            49,
            138,
            11
        ),
        RGB(
            21,
            109,
            209
        ),
        RGB(
            166,
            165,
            151
        ),
        RGB(
            224,
            220,
            193
        ),
        RGB(
            212,
            235,
            145
        ),
        RGB(
            223,
            240,
            173
        ),
        RGB(
            59,
            130,
            5
        ),
        RGB(
            121,
            140,
            122
        ),
        RGB(
            145,
            161,
            145
        ),
        RGB(
            56,
            82,
            42
        ),
        RGB(
            74,
            99,
            61
        ),
        RGB(
            167,
            199,
            157
        ),
        RGB(
            175,
            181,
            69
        ),
        RGB(
            171,
            170,
            114
        ),
        RGB(
            115,
            66,
            44
        ),
        RGB(
            107,
            81,
            70
        ),
        RGB(
            143,
            114,
            101
        )
    };

    static Layers::GenLayer *Init()
    {
        Layers::GenLayer *island = new Layers::Island(1);

        Layers::GenLayer *fuzzy_zoom = new Layers::FuzzyZoom(2000, island);

        Layers::GenLayer *add_island1 = new Layers::AddIsland(1, fuzzy_zoom);

        Layers::GenLayer *zoom1 = new Layers::Zoom(2001, add_island1);

        Layers::GenLayer *add_island2 = new Layers::AddIsland(2, zoom1);

        add_island2 = new Layers::AddIsland(50, add_island2);

        add_island2 = new Layers::AddIsland(70, add_island2);

        Layers::GenLayer *remove_too_much_ocean = new Layers::RemoveTooMuchOcean(2, add_island2);

        Layers::GenLayer *add_snow = new Layers::AddSnow(2, remove_too_much_ocean);

        Layers::GenLayer *add_island3 = new Layers::AddIsland(3, add_snow);

        Layers::GenLayer *edge_cool_warm = new Layers::EdgeCoolWarm(2, add_island3);

        Layers::GenLayer *edge_heat_ice = new Layers::EdgeHeatIce(2, edge_cool_warm);

        Layers::GenLayer *edge_special = new Layers::EdgeSpecial(3, edge_heat_ice);

        Layers::GenLayer *zoom2 = new Layers::Zoom(2002, edge_special);

        zoom2 = new Layers::Zoom(2003, zoom2);

        Layers::GenLayer *add_island4 = new Layers::AddIsland(4, zoom2);

        Layers::GenLayer *mushroom = new Layers::AddMushroomIsland(5, add_island4);

        Layers::GenLayer *deep_ocean = new Layers::DeepOcean(4, mushroom);

        Layers::GenLayer *biome = new Layers::Biome(200, deep_ocean);

        Layers::GenLayer *river_init = new Layers::RiverInit(100, deep_ocean);

        Layers::GenLayer *river_zoom1 = new Layers::Zoom(1000, river_init);

        river_zoom1 = new Layers::Zoom(1001, river_zoom1);

        Layers::GenLayer *zoom3 = new Layers::Zoom(1000, biome);

        zoom3 = new Layers::Zoom(1001, zoom3);

        Layers::GenLayer *biome_edge = new Layers::BiomeEdge(1000, zoom3);

        Layers::GenLayer *hills = new Layers::Hills(1000, biome_edge, river_zoom1);

        Layers::GenLayer *river_zoom2 = new Layers::Zoom(1000, river_zoom1);

        river_zoom2 = new Layers::Zoom(1001, river_zoom2);

        river_zoom2 = new Layers::Zoom(1002, river_zoom2);

        river_zoom2 = new Layers::Zoom(1003, river_zoom2);

        Layers::GenLayer *zoom4 = new Layers::Zoom(1000, hills);

        Layers::GenLayer *add_island5 = new Layers::AddIsland(3, zoom4);

        Layers::GenLayer *zoom5 = new Layers::Zoom(1001, add_island5);

        Layers::GenLayer *shore = new Layers::Shore(1000, zoom5);

        Layers::GenLayer *zoom6 = new Layers::Zoom(1002, shore);

        zoom6 = new Layers::Zoom(1003, zoom6);

        Layers::GenLayer *smooth = new Layers::Smooth(1000, zoom6);

        Layers::GenLayer *river = new Layers::River(1, river_zoom2);

        Layers::GenLayer *river_smooth = new Layers::Smooth(1000, river);

        Layers::GenLayer *river_mix = new Layers::RiverMix(100, smooth, river_smooth);
        return river_mix;
    }
}


void RefreshBiomes(int x, int y, int w, int h, unsigned char *pImgBuf)
{
    static Layers::GenLayer* topLayer = nullptr;
    if (topLayer == nullptr)
    {
        topLayer = Vis::Init();
    }
        
    tarray ptr = topLayer->get_ints(x, y, w, h);
    for (int oy = 0; oy < h; ++oy)
    {
        for (int ox = 0; ox < w; ++ox)
        {
            const Vis::RGB &rgb = Vis::biome_colors[ptr.p(ox, oy)];
            pImgBuf[(oy * w + ox) * 4] = rgb.c[0];
            pImgBuf[(oy * w + ox) * 4 + 1] = rgb.c[1];
            pImgBuf[(oy * w + ox) * 4 + 2] = rgb.c[2];
            pImgBuf[(oy * w + ox) * 4 + 3] = 255;
        }
    }
}
