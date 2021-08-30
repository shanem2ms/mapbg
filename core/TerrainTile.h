#pragma once

#include <map>
#include <set>
#include "SceneItem.h"
#include "OctTile.h"

namespace sam
{
    class TerrainTile : public SceneItem
    {
        static const int SquarePtsCt = 256;
        static const int OverlapPtsCt = 64;
        static const int TotalPtsCt = SquarePtsCt + OverlapPtsCt * 2;

        int m_image;
        Vec2f m_vals;
        Loc m_l;
        bool m_needRecalc;
        float m_pts[TotalPtsCt * TotalPtsCt];
        Vec2f m_maxdh;
        Vec2f m_mindh;
        bgfx::TextureHandle m_tex[2];
        bgfx::TextureHandle m_terrain;
        bgfx::TextureHandle m_rbTex;
        int m_texpingpong;
        int m_buildFrame;
        bool m_dataready;
        std::vector<float> m_heightData;
        bgfx::UniformHandle m_uparams;

    public:
        float distFromCam;
    public:
        void Draw(DrawContext& ctx) override;
        TerrainTile(const Loc& l);
        ~TerrainTile();

        const float* Pts() const { return m_pts; }
        void SetImage(int image)
        {
            m_image = image;
        }
        gmtl::AABoxf GetBounds() const override;
        void SetVals(const Vec2f& v)
        {
            m_vals = v;
        }
        void Decomission();

        float GetGroundHeight(const gmtl::Point3f& pt) const;

    private:
        void NoiseGen();
        void ProceduralBuild(DrawContext& ctx);
    };


}