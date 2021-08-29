#pragma once
#include "Engine.h"
#include "UIControl.h"
#include "World.h"

namespace sam
{
class Application
{
    World m_world;
    Engine m_engine;
    UIManager m_uiMgr;
    int m_width;
    int m_height;
    float m_touchDownX;
    float m_touchDownY;
    int m_frameIdx;

public:    
    UIManager& UIMgr();
    Application();
    static Application& Inst();
    void TouchDown(float x, float y, int touchId);
    void TouchDrag(float x, float y, int touchId);
    void TouchUp(int touchId);
    void KeyDown(int keyId);
    void KeyUp(int keyId);
    void Resize(int w, int h);
    void Tick(float time);
    void LoadResources(DrawContext & nvg);
    void Draw(DrawContext & nvg);
};

}