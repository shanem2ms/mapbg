#include "SceneItem.h"
#include <bgfx/bgfx.h>

using namespace gmtl;

struct PosTexcoordVertex
{
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    };

    static bgfx::VertexLayout ms_layout;
};

struct Cube
{
    static void init()
    {
        if (isInit)
            return;
        PosTexcoordVertex::init();


        static PosTexcoordVertex s_cubeVertices[] =
        {
            {-1.0f,  1.0f,  1.0f,  0.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f,  1.0f,  1.0f},
            {-1.0f, -1.0f,  1.0f,  0.0f,  0.0f},
            { 1.0f, -1.0f,  1.0f,  1.0f,  0.0f},

            {-1.0f,  1.0f, -1.0f,  0.0f,  1.0f},
            { 1.0f,  1.0f, -1.0f,  1.0f,  1.0f},
            {-1.0f, -1.0f, -1.0f,  0.0f,  0.0f},
            { 1.0f, -1.0f, -1.0f,  1.0f,  0.0f},

            {-1.0f,  1.0f,  1.0f, 0.0f,  1.0f},
            {-1.0f,  1.0f, -1.0f, 1.0f,  1.0f},
            {-1.0f, -1.0f,  1.0f, 0.0f,  0.0f},
            {-1.0f, -1.0f, -1.0f, 1.0f,  0.0f},

            { 1.0f,  1.0f,  1.0f, 0.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f, 1.0f,  1.0f},
            { 1.0f,  1.0f, -1.0f, 0.0f,  0.0f},
            { 1.0f, -1.0f, -1.0f, 1.0f,  0.0f},

            {-1.0f,  1.0f,  1.0f, 0.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f, 1.0f,  1.0f},
            {-1.0f,  1.0f, -1.0f, 0.0f,  0.0f},
            { 1.0f,  1.0f, -1.0f, 1.0f,  0.0f},

            {-1.0f, -1.0f,  1.0f, 0.0f,  1.0f},
            {-1.0f, -1.0f, -1.0f, 1.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f, 0.0f,  0.0f},
            { 1.0f, -1.0f, -1.0f, 1.0f,  0.0f},
        };

        static const uint16_t s_cubeIndices[] =
        {
             0,  1,  2, // 0
             1,  3,  2,

             4,  6,  5, // 2
             5,  6,  7,

             8, 10,  9, // 4
             9, 10, 11,

            12, 14, 13, // 6
            14, 15, 13,

            16, 18, 17, // 8
            18, 19, 17,

            20, 22, 21, // 10
            21, 22, 23,
        };

        vbh = bgfx::createVertexBuffer(
            bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices))
            , PosTexcoordVertex::ms_layout
        );

        ibh = bgfx::createIndexBuffer(
            bgfx::makeRef(s_cubeIndices, sizeof(s_cubeIndices))
        );

        isInit = true;
    }

    static bgfx::VertexBufferHandle vbh;
    static bgfx::IndexBufferHandle ibh;
    static bool isInit;
};


struct Grid
{
    static void init();

    static bgfx::VertexBufferHandle vbh;
    static bgfx::IndexBufferHandle ibh;
    static std::vector<PosTexcoordVertex> vertices;
    static std::vector<uint16_t> indices;
    static bool isInit;
};

