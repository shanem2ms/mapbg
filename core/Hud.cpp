#include "StdIncludes.h"
#include "World.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "Hud.h"
#include "imgui.h"
#include "TerrainTileSelection.h"
#define NOMINMAX

namespace sam
{
    void Hud::Initialize(DrawContext& nvg)
    {
        m_shader = Engine::Inst().LoadShader("vs_hud.bin", "fs_hud.bin");
    }

    extern int g_nearTiles;
    extern int g_farTiles;
    extern int nOctTilesTotal;
    extern int nOctTilesDrawn;


	void Hud::Draw(DrawContext& ctx)
	{        
        Matrix44f m =
            ctx.m_mat * CalcMat();

        //bgfx::setTransform(m.getData());
        Quad::init();

        // Set vertex and index buffer.
        bgfx::setVertexBuffer(0, Quad::vbh);
        bgfx::setIndexBuffer(Quad::ibh);
        uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA;
        // Set render states.l
        bgfx::setState(state);
        bgfx::submit(0, m_shader);

        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 2, 0x0f, "Terrain Tiles [%d]", TerrainTileSelection::sNumTiles.load());
        bgfx::dbgTextPrintf(0, 3, 0x0f, "Oct Tiles [%d, %d, %d]", OctTileSelection::sNumTiles.load(), nOctTilesTotal, nOctTilesDrawn);
        bgfx::dbgTextPrintf(0, 6, 0x0f, "Near Tiles [%d]", g_nearTiles);
        bgfx::dbgTextPrintf(0, 7, 0x0f, "Far Tiles [%d]", g_farTiles);

        Engine& e = Engine::Inst();
        Camera::Fly la = e.Cam().GetFly();
        bgfx::dbgTextPrintf(0, 4, 0x0f, "Cam [%f %f %f]", la.pos[0], la.pos[1], la.pos[2]);
	}

	AABoxf Hud::GetBounds() const
	{
		return AABoxf();
	}
}
