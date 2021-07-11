#pragma once
#include "Engine.h"
#include "UIControl.h"
#include "Board.h"

namespace sam
{
class Application
{
    Board m_board;
    Engine m_engine;
    UIManager m_uiMgr;
    int m_width;
    int m_height;
    float m_touchDownX;
    float m_touchDownY;

public:    
    UIManager& UIMgr();
    Application();
    static Application& Inst();
    void TouchDown(float x, float y, int touchId);
    void TouchDrag(float x, float y, int touchId);
    void TouchUp(int touchId);
    void Resize(int w, int h);
    void Tick(float time);
    void LoadResources(DrawContext & nvg);
    void Draw(DrawContext & nvg);
};

}