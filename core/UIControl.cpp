#include "UIControl.h"
#include "Engine.h"

namespace sam
{

    UIControl::UIControl(float x, float y, float w, float h) :
        m_x(x),
        m_y(y),
        m_width(w),
        m_height(h),
        m_isInit(false),
        m_background(1.0f, 1.0f, 1.0f, 1.0f)
    {

    }

    void UIControl::SetBackgroundColor(const Vec4f& color)
    {
        m_background = color;
    }
    void UIControl::SetBorderColor(const Vec4f& color)
    {
        m_border = color;
    }

    void UIControl::Update(const std::shared_ptr<SceneGroup>& group, DrawContext& ctx)
    {
        if (!m_isInit)
        {
            Initialize(group, ctx);
            m_isInit = true;
        }
    }

    bool UIControl::IsHit(float x, float y, int touchId)
    {
        return x >= m_x && x < (m_x + m_width) &&
            y >= m_y && y < (m_y + m_height);
    }


    void UIManager::AddControl(std::shared_ptr<UIControl> ctrl)
    {
        m_controls.push_back(ctrl);
    }

    bool UIManager::TouchDown(float x, float y, int touchId)
    {
        bool handled = false;
        for (auto uictrl : m_controls)
        {
            if (uictrl->IsHit(x, y, touchId) &&
                uictrl->TouchDown(x, y, touchId))
            {
                m_capturedCtrl = uictrl;
                handled = true;
                break;
            }
        }

        return handled;
    }

    bool UIManager::TouchDrag(float x, float y, int touchId)
    {
        if (m_capturedCtrl != nullptr)
        {
            m_capturedCtrl->TouchDrag(x, y, touchId);
            return true;
        }
        return false;
    }

    void UIManager::Update(Engine& engine, int w, int h, DrawContext& ctx)
    {
        if (m_uiGroup == nullptr)
        {
            m_uiGroup = std::make_shared<SceneGroup>();
            m_uiGroup->SetOffset(Point3f(0, 0, w / 2));
            engine.Root()->AddItem(m_uiGroup);
        }
        for (auto ctrl : m_controls)
        {
            ctrl->Update(m_uiGroup, ctx);
        }
    }

    bool UIManager::TouchUp(int touchId)
    {
        if (m_capturedCtrl != nullptr)
        {
            m_capturedCtrl->TouchUp(touchId);
            m_capturedCtrl = nullptr;
            return true;
        }

        return false;
    }

    UIButton::UIButton(const std::string& text, float x, float y, float w, float h) :
        UIControl(x, y, w, h),
        m_text(text)
    {
        m_background = Vec4f(0.6f, 0.6f, 0.6f, 1.0f);
        m_border = Vec4f(0.8f, 0.8f, 0.8f, 1.0f);
        m_pressedColor = Vec4f(0.9f, 0.6f, 0.6f, 1.0f);
    }

    void UIButton::Initialize(const std::shared_ptr<SceneGroup>& group, DrawContext& ctx)
    {
        std::shared_ptr<SceneGroup> grp = std::make_shared<SceneGroup>();
        Vec3f scl = Vec3f(m_width * 0.5f, m_height * 0.5f, 1);
        grp->SetScale(scl);
        grp->SetOffset(Point3f(m_x + scl[0], m_y + scl[1], 0));

        m_rect = std::make_shared<SceneRect>();
        m_rect->SetColor(m_background);
        m_rect->SetStroke(m_border, 3.0f);
        grp->AddItem(m_rect);
        std::shared_ptr<SceneText> text = std::make_shared<SceneText>();
        text->SetOffset(Point3f(0, 0.5f, 0));
        text->SetText(m_text);
        text->SetColor(Vec4f(0, 0, 0, 1));
        text->SetFont("roboto", m_height * 0.6f);
        grp->AddItem(text);

        group->AddItem(grp);
    }

    bool UIButton::TouchDown(float x, float y, int touchId)
    {
        m_rect->SetColor(m_pressedColor);
        if (m_pressed != nullptr)
            m_pressed(touchId);
        return true;
    }

    bool UIButton::TouchDrag(float x, float y, int touchId)
    {
        return true;
    }

    bool UIButton::TouchUp(int touchId)
    {
        m_rect->SetColor(m_background);
        return true;
    }
    void UIButton::OnPressed(const UIButton::PressedFunc& pressed)
    {
        m_pressed = pressed;
    }

}