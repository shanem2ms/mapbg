#include "Mesh.h"

bgfx::VertexLayout PosTexcoordVertex::ms_layout;

bool Cube::isInit = false;
bgfx::VertexBufferHandle Cube::vbh;
bgfx::IndexBufferHandle Cube::ibh;


template <int N> void Grid<N>::init()
{
    if (isInit)
        return;
    PosTexcoordVertex::init();
    const int size = 16;
    vertices.resize(size * size);
    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            PosTexcoordVertex& v = vertices[y * size + x];
            v.m_x = ((float)(x) / (float)(size - 1)) * 2 - 1;
            v.m_z = ((float)(y) / (float)(size - 1)) * 2 - 1;
            v.m_y = 0;
            v.m_u = ((float)(x) / (float)(size - 1));
            v.m_v = ((float)(y) / (float)(size - 1));

            if (x < size - 1 &&
                y < size - 1)
            {
                int idx = y * size + x;
                indices.push_back(idx + 1);
                indices.push_back(idx + size);
                indices.push_back(idx);

                indices.push_back(idx + size + 1);
                indices.push_back(idx + size);
                indices.push_back(idx + 1);
            }
        }
    }

    vbh = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices.data(), vertices.size() * sizeof(PosTexcoordVertex)),
        PosTexcoordVertex::ms_layout
    );

    ibh = bgfx::createIndexBuffer(
        bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
    );

    isInit = true;
}

template <int N> bool Grid<N>::isInit = false;
template <int N> bgfx::VertexBufferHandle Grid<N>::vbh;
template <int N> bgfx::IndexBufferHandle Grid<N>::ibh;
template <int N> std::vector<PosTexcoordVertex> Grid<N>::vertices;
template <int N> std::vector<uint16_t> Grid<N>::indices;
template class Grid<16>;
