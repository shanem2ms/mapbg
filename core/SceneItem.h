#pragma once
typedef struct NVGcontext NVGcontext;
#include "StdIncludes.h"
#include <gmtl/gmtl.h>
#include <gmtl/Matrix.h>
#include <gmtl/Point.h>
#include <gmtl/Quat.h>
#include <gmtl/Vec.h>
#include <gmtl/AABox.h>
#include <gmtl/AABoxOps.h>
#include <gmtl/Plane.h>
#include <gmtl/PlaneOps.h>
#include <gmtl/Frustum.h>
#include <functional>

using namespace gmtl;

namespace sam
{
    class Camera;
    class World;

    struct DrawContext
    {
        bgfx::ProgramHandle m_pgm;
        Matrix44f m_mat;
        bgfx::UniformHandle m_texture;
        bgfx::UniformHandle m_gradient;
        World* m_pWorld;
        int m_nearfarpassIdx;
        float m_nearfar[3];
        int m_curviewIdx;
        int m_frameIdx;
        int m_numGpuCalcs;
    };


    class Camera
    {
        gmtl::Matrix44f m_proj;
        mutable gmtl::Matrix44f m_view;
        float m_aspect;
        float m_near;
        float m_far;

    public:

        struct LookAt
        {
            LookAt() : pos(0, 0, 0),
                tilt(0),
                dist(0) {}

            Point3f pos;
            float tilt;
            float dist;
        };

        struct Fly
        {
            Fly() : pos(0, 0, 0),
                dir(0, 0) {}

            Point3f pos;
            Vec2f dir;

            void GetDirs(Vec3f& right, Vec3f& up, Vec3f& forward) const;
        };

        int m_mode;
        LookAt m_lookat;
        Fly m_fly;
        mutable bool m_viewdirty; 


        Camera();
        void Update(int w, int h);
        void SetNearFar(float near, float far)
        {
            m_near = near; m_far = far;
        }

        Frustumf GetFrustum(float near, float far) const;

        void SetLookat(const LookAt& la)
        {
            m_lookat = la;
        }

        const LookAt& GetLookat() const { return m_lookat; }

        void SetFly(const Fly& la)
        {
            m_fly = la;
            m_viewdirty = true;
        }

        const Fly& GetFly() const { return m_fly; }

        gmtl::Matrix44f GetPerspectiveMatrix(float near, float far) const;

        const gmtl::Matrix44f& PerspectiveMatrix() const
        {
            return m_proj;
        }

        const gmtl::Matrix44f& ViewMatrix() const;
    };

    class SceneItem
    {
    protected:
        Point3f m_offset;
        Vec3f m_scale;
        Quatf m_rotate;
        Vec4f m_color;
        Vec4f m_strokeColor;
        float m_strokeWidth;
        bool m_isInitialized;
        bool m_enabled;

        SceneItem();

        Matrix44f CalcMat() const;
    public:
        void SetAnchor(const Point3f& p)
        {

        }

        void SetOffset(const Point3f& p)
        {
            m_offset = p;
        }

        void SetEnabled(bool enabled)
        {
            m_enabled = enabled;
        }

        void SetStroke(const Vec4f& color, float width)
        {
            m_strokeColor = color;
            m_strokeWidth = width;
        }
        const Point3f& GetOffset() const
        {
            return m_offset;
        }
        void SetScale(const Vec3f& s)
        {
            m_scale = s;
        }
        void SetRotate(const Quatf& r)
        {
            m_rotate = r;
        }
        void SetColor(const Vec3f& col)
        {
            SetColor(Vec4f(col[0], col[1], col[2], 1));
        }
        void SetColor(const Vec4f& col)
        {
            m_color = col;
        }
        
        void DoDraw(DrawContext& ctx);

        virtual AABoxf GetBounds() const
        { return AABoxf(); }
    protected:

        virtual void Initialize(DrawContext& nvg) {}
        virtual void Draw(DrawContext& ctx) = 0;
    };


    class SceneGroup : public SceneItem
    {
    protected:
        std::vector<std::shared_ptr<SceneItem>> m_sceneItems;
        std::function<bool(DrawContext& ctx)> m_beforeDraw;
        std::function<void(DrawContext& ctx)> m_afterDraw;

    public:
        void AddItem(const std::shared_ptr<SceneItem>& item)
        {
            m_sceneItems.push_back(item);
        }

        void BeforeDraw(std::function<bool(DrawContext& ctx)> f)
        { m_beforeDraw = f; }

        void AfterDraw(std::function<void(DrawContext& ctx)> f)
        { m_afterDraw = f; }

        void Clear()
        {
            m_sceneItems.clear();
        }

        void Draw(DrawContext& ctx) override;

        AABoxf GetBounds() const override;
    };

    class SceneText : public SceneItem
    {
    protected:

        std::string m_text;
        std::string m_font;
        float m_size;
    public:

        void SetText(const std::string& text);
        void SetFont(const std::string& fontname, float size);
        void Draw(DrawContext& ctx) override;
        AABoxf GetBounds() const override;
    };

    class SceneRect : public SceneItem
    {
    protected:

    public:

        void Draw(DrawContext& ctx) override;
        AABoxf GetBounds() const override;
    };

}