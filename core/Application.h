#pragma once

#include <memory>

namespace sam
{

class World;
class Engine;
class UIManager;
struct DrawContext;
class Application
{
    std::unique_ptr<World> m_world;
    std::unique_ptr<Engine> m_engine;
    std::unique_ptr<UIManager> m_uiMgr;
    int m_width;
    int m_height;
    float m_touchDownX;
    float m_touchDownY;
    int m_frameIdx;

public:    
    UIManager& UIMgr();
    Application();
    ~Application();
    static Application& Inst();
    void TouchDown(float x, float y, int touchId);
    void TouchDrag(float x, float y, int touchId);
    void TouchUp(int touchId);
    void KeyDown(int keyId);
    void KeyUp(int keyId);
    void Resize(int w, int h);
    void Tick(float time);
    void Draw();
};

}