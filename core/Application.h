#pragma once

#include <memory>
#include <string>

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
    std::string m_documentsPath;

public:    
    UIManager& UIMgr();
    Application();
    ~Application();
    static Application& Inst();
    int FrameIdx() const { return m_frameIdx; }
    void TouchDown(float x, float y, int touchId);
    void TouchDrag(float x, float y, int touchId);
    void TouchUp(int touchId);
    void KeyDown(int keyId);
    void KeyUp(int keyId);
    void Resize(int w, int h);
    void Tick(float time);
    void Draw();
    void SetDocPath(const char *folder);
    const std::string &Documents() const
    { return m_documentsPath; }
};

}
