#include "StdIncludes.h"
#include "Application.h"
#include <bgfx/bgfx.h>
#include "Engine.h"
#include "UIControl.h"
#include "World.h"
#include "imgui.h"
#include <chrono>

#define WATCHDOGTHREAD 0

namespace sam
{
#ifdef SAM_COROUTINES
    co::static_thread_pool g_threadPool(8);
#endif
    static Application* s_pInst = nullptr;
    static std::thread sWatchdogThread;
    static std::chrono::steady_clock::time_point sFrameStart;

#if WATCHDOGTHREAD
    static bool sWatchDogCheckEnabled = false;
    void WatchDogFunc();
    using namespace std::chrono_literals;
#endif

int g_buttonDown = 0;
    Application::Application() :
        m_height(0),
        m_touchDown(0, 0),
        m_touchPos(0, 0),
        m_width(0),
        m_frameIdx(0),
        m_buttonDown(false)
    {
        s_pInst = this;
        m_engine = std::make_unique<Engine>();
        m_world = std::make_unique<World>();
        m_uiMgr = std::make_unique<UIManager>();
#if WATCHDOGTHREAD
        sWatchdogThread = std::thread(WatchDogFunc);
#endif
    }

#if WATCHDOGTHREAD
    void WatchDogFunc()
    {
        while (true)
        {
            std::this_thread::sleep_for(1ms);
            auto elapsed = std::chrono::high_resolution_clock::now()
                - sFrameStart;
            if (sWatchDogCheckEnabled && elapsed > 20ms)
            {
                __debugbreak();
            }
        }
    }
#endif

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
        m_touchPos = m_touchDown = gmtl::Vec2f(x, y);

        g_buttonDown = m_buttonDown = 1;
        if (!m_uiMgr->TouchDown(x, y, touchId))
            m_world->TouchDown(x, y, touchId);
    }

    void Application::TouchMove(float x, float y, int touchId)
    {
        m_touchPos = gmtl::Vec2f(x, y);
        if (!m_uiMgr->TouchDrag(x, y, touchId))
            m_world->TouchDrag(x, y, touchId);
    }

    void Application::TouchUp(int touchId)
    {
        g_buttonDown = m_buttonDown = 0;
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

    static int sButtonStates[256];
    
    void Application::Initialize(const char *folder)
    {
        m_documentsPath = folder;
        std::string dbPath = m_documentsPath + "/testlvl";
        m_world->Open(dbPath);
        imguiCreate();
        memset(sButtonStates, 0, sizeof(sButtonStates));
    }

    const float Pi = 3.1415297;
    float g_Fps = 0;
    int counter = 0;
    void Application::Draw()
    {
        sFrameStart = std::chrono::high_resolution_clock::now();
        sam::DrawContext ctx;
        ctx.m_nearfar[0] = 0.1f;
        ctx.m_nearfar[1] = 25.0f;
        ctx.m_nearfar[2] = 100.0f;
        ctx.m_frameIdx = m_frameIdx;
        ctx.m_pWorld = m_world.get();
        ctx.m_numGpuCalcs = 0;
        m_uiMgr->Update(*m_engine, m_width, m_height, ctx);
        m_world->Update(*m_engine, ctx);

        bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
        bgfx::setViewRect(1, 0, 0, uint16_t(m_width), uint16_t(m_height));
        m_engine->Draw(ctx);

        imguiBeginFrame(m_touchPos[0]
            , m_touchPos[1]
            , m_buttonDown
            , 0
            , uint16_t(m_width)
            , uint16_t(m_height)
        );
        
        const int btnSize = 150;
        const int btnSpace = 10;
        ImGui::SetNextWindowPos(
            ImVec2(m_width - btnSize*4, m_height - btnSize*3)
            , ImGuiCond_FirstUseEver
        );

        int buttonsThisFrame[256];
        memset(buttonsThisFrame, 0, sizeof(buttonsThisFrame));
        ImGui::Begin("make window", nullptr, 
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

        ImGui::SetCursorPos(ImVec2(btnSize + btnSpace * 2,0));
        ImGui::Button(ICON_FA_CHEVRON_UP, ImVec2(btnSize, btnSize));
        buttonsThisFrame['W'] = ImGui::IsItemActive();
        ImGui::SetCursorPos(ImVec2(btnSize + btnSpace * 2, btnSize + btnSpace));
        ImGui::Button(ICON_FA_CHEVRON_DOWN, ImVec2(btnSize, btnSize));
        buttonsThisFrame['S'] = ImGui::IsItemActive();
        ImGui::SetCursorPos(ImVec2(btnSize * 2 + btnSpace * 4, btnSize / 2));
        ImGui::Button(ICON_FA_CHEVRON_RIGHT, ImVec2(btnSize, btnSize));
        buttonsThisFrame['D'] = ImGui::IsItemActive();
        ImGui::SetCursorPos(ImVec2(0, btnSize / 2));
        ImGui::Button(ICON_FA_CHEVRON_LEFT, ImVec2(btnSize, btnSize));
        buttonsThisFrame['A'] = ImGui::IsItemActive();
        ImGui::End();

        imguiEndFrame();
        m_frameIdx = bgfx::frame() + 1;
        auto elapsed = std::chrono::high_resolution_clock::now() - sFrameStart;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        g_Fps = (float)1000000.0f / microseconds;

        for (int i = 0; i < 256; ++i)
        {
            if (buttonsThisFrame[i] != sButtonStates[i])
            {
                if (buttonsThisFrame[i] > 0)
                    m_world->KeyDown(i);
                else
                    m_world->KeyUp(i);
            }
        }
        memcpy(sButtonStates, buttonsThisFrame, sizeof(sButtonStates));
#if WATCHDOGTHREAD
        if (elapsed < 20ms)
        {
            if (counter++ == 100)
                sWatchDogCheckEnabled = true;
        }
        else
            counter = 0;
#endif
    }
    Application::~Application()
    {

    }
}
