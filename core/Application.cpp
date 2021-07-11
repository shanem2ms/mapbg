#include "Application.h"
#include <bgfx/bgfx.h>

namespace sam
{
    static Application* s_pInst = nullptr;

    Application::Application() :
        m_height(0),
        m_touchDownX(0),
        m_touchDownY(0),
        m_width(0)
    {
        s_pInst = this;
    }

    Application& Application::Inst()
    {
        return *s_pInst;
    }


    UIManager& Application::UIMgr()
    {
        return m_uiMgr;
    }

    void Application::TouchDown(float x, float y, int touchId)
    {
        m_touchDownX = x;
        m_touchDownY = y;

        if (!m_uiMgr.TouchDown(x, y, touchId))
            m_board.TouchDown(x, y, touchId);
    }

    void Application::TouchDrag(float x, float y, int touchId)
    {
        if (!m_uiMgr.TouchDrag(x, y, touchId))
            m_board.TouchDrag(x, y, touchId);
    }

    void Application::TouchUp(int touchId)
    {
        if (!m_uiMgr.TouchUp(touchId))
            m_board.TouchUp(touchId);
    }

    void Application::Resize(int w, int h)
    {
        m_width = w;
        m_height = h;
        m_engine.Resize(w, h);
        m_board.Layout(w, h);
    }
    void Application::Tick(float time)
    {
        m_engine.Tick(time);
    }

    const float Pi = 3.1415297;

    void Application::Draw(DrawContext& nvg)
    {
        m_uiMgr.Update(m_engine, m_width, m_height, nvg);
        m_board.Update(m_engine, nvg);

        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x303030ff
            , 1.0f
            , 0
        );

        gmtl::Matrix44f proj = m_engine.Cam().PerspectiveMatrix();
        gmtl::Matrix44f view = m_engine.Cam().ViewMatrix();

        bgfx::setViewTransform(0, view.getData(), proj.getData());
        bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

        m_engine.Draw(nvg);
        bgfx::frame();
    }

    void Application::LoadResources(DrawContext& nvg)
    {
        m_engine.LoadResources(nvg);
    }

}