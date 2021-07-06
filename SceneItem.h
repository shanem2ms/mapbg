#pragma once
typedef struct NVGcontext NVGcontext;
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
#include <bgfx/bgfx.h>
#include "HSLColor.h"

using namespace gmtl;
class Camera;

struct DrawContext
{
    bgfx::ProgramHandle m_pgm;
    Matrix44f m_mat;
    bgfx::UniformHandle m_texture;
    bgfx::UniformHandle m_gradient;
    bgfx::ProgramHandle m_compute;
};


class Camera
{
    gmtl::Matrix44f m_proj;
    gmtl::Matrix44f m_view;


public:

    struct LookAt
    {
        LookAt() : pos(0,0,0),
            tilt(0),
            dist(0) {}

        Point3f pos;
        float tilt;
        float dist;
    };

    LookAt m_lookat;
    Camera();
    void Update(int w, int h);

    Frustumf GetFrustum() const;

    void SetLookat(const LookAt& la)
    {
        m_lookat = la;
    }

    const LookAt&GetLookat() const { return m_lookat; }

    const gmtl::Matrix44f &PerspectiveMatrix() const
    { return m_proj; }

    const gmtl::Matrix44f &ViewMatrix() const
    { return m_view; }
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

    void SetStroke(const Vec4f &color, float width)
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
    virtual void Draw(DrawContext& ctx) = 0;

    virtual AABoxf GetBounds() const = 0;
};


class SceneGroup : public SceneItem
{
protected:
    std::vector<std::shared_ptr<SceneItem>> m_sceneItems;

public:
    void AddItem(const std::shared_ptr<SceneItem>& item)
    {
        m_sceneItems.push_back(item);
    }

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