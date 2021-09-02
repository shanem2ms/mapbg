#include "StdIncludes.h"
#include "Application.h"
#include <bgfx/bgfx.h>
#include "Engine.h"
#include "UIControl.h"
#include "World.h"

namespace sam
{
    co::static_thread_pool g_threadPool(8);
    static Application* s_pInst = nullptr;

    Application::Application() :
        m_height(0),
        m_touchDownX(0),
        m_touchDownY(0),
        m_width(0),
        m_frameIdx(0)
    {
        s_pInst = this;
        m_engine = std::make_unique<Engine>();
        m_world = std::make_unique<World>();
        m_uiMgr = std::make_unique<UIManager>();
    }

    Application& Application::Inst()
    {
        return *s_pInst;
    }


    UIManager& Application::UIMgr()
    {
        return *m_uiMgr;
    }

    void Application::TouchDown(float x, float y, int touchId)
    {
        m_touchDownX = x;
        m_touchDownY = y;

        if (!m_uiMgr->TouchDown(x, y, touchId))
            m_world->TouchDown(x, y, touchId);
    }

    void Application::TouchDrag(float x, float y, int touchId)
    {
        if (!m_uiMgr->TouchDrag(x, y, touchId))
            m_world->TouchDrag(x, y, touchId);
    }

    void Application::TouchUp(int touchId)
    {
        if (!m_uiMgr->TouchUp(touchId))
            m_world->TouchUp(touchId);
    }

    void Application::KeyDown(int keyId)
    {
        m_world->KeyDown(keyId);
    }

    void Application::KeyUp(int keyId)
    {
        m_world->KeyUp(keyId);
    }

    void Application::Resize(int w, int h)
    {
        m_width = w;
        m_height = h;
        m_engine->Resize(w, h);
        m_world->Layout(w, h);
    }
    void Application::Tick(float time)
    {
        m_engine->Tick(time);
    }    

    const float Pi = 3.1415297;

    void Application::Draw()
    {
        sam::DrawContext ctx;
        m_uiMgr->Update(*m_engine, m_width, m_height, ctx);
        m_world->Update(*m_engine, ctx);

        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x303030ff
            , 1.0f
            , 0
        );

        bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
        ctx.m_frameIdx = m_frameIdx;
        m_engine->Draw(ctx);
        m_frameIdx = bgfx::frame() + 1;
    }
    Application::~Application()
    {

    }
}
