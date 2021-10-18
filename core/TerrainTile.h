#pragma once

#include <map>
#include <set>
#include "SceneItem.h"
#include "OctTile.h"

namespace sam
{
    class World;
    class TerrainTile
    {
    public:
        static const int SquarePtsCt = 256;
        static const int OverlapPtsCt = SquarePtsCt/4;
        static const int TotalPtsCt = SquarePtsCt + OverlapPtsCt * 2;
    private:
        int m_image;
        Vec2f m_vals;
        Loc m_l;
        bool m_needRecalc;
        std::vector<float> m_pts;
        std::vector<float> m_heightData;
        Vec2f m_maxdh;
        Vec2f m_mindh;
        bgfxh<bgfx::TextureHandle> m_tex[2];
        bgfxh<bgfx::TextureHandle> m_terrain;
        bgfxh<bgfx::TextureHandle> m_rbTex;
        int m_texpingpong;
        int m_buildFrame;
        int m_buildStep;
        bool m_dataready;
        AABoxf m_heightBbox;
        std::shared_ptr<TerrainTile> m_parent;
        int m_lastUsedframeIdx;
        
        static bgfx::UniformHandle m_texture;
        static bgfx::UniformHandle m_uparams;
        static bgfx::UniformHandle m_vparams;
        static bgfx::ProgramHandle m_erosion;
        static bgfx::ProgramHandle m_copysect;
        static bgfx::ProgramHandle m_csnoise;
        static bgfx::ProgramHandle m_cscopyparent;
    public:
        float distFromCam;
    public:
        TerrainTile(const Loc& l, std::shared_ptr<TerrainTile> parent);
        TerrainTile(const Loc& l, const std::string &val);
        ~TerrainTile();

        const bgfxh<bgfx::TextureHandle>& GetTerrain();
        void SetLastUseFrame(int frameIdx)
        { m_lastUsedframeIdx = frameIdx; }

        int GetLastUsedFrame() const {
            return m_lastUsedframeIdx; }

        const AABoxf& GetBounds() const {
            return m_heightBbox;
        }

        bool IsDataReady() const { return m_dataready; }
        const float* Pts() const { return m_heightData.data(); }
        void SetImage(int image)
        {
            m_image = image;
        }
        void SetVals(const Vec2f& v)
        {
            m_vals = v;
        }

        float GetGroundHeight(const gmtl::Point3f& pt) const;
        bool Build(DrawContext& ctx);
    private:
    };


}