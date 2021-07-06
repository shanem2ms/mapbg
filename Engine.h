#pragma once

#include "SceneItem.h"
struct DrawContext;

class Animation
{
protected:
    float m_startTime;
    Animation();
    virtual bool Tick(float time) = 0;

public:
    bool ProcessTick(float fullTime);
};

class Engine
{
    int m_w;
    int m_h;
    Camera m_camera;

    std::shared_ptr<SceneGroup> m_root;
    std::vector<std::shared_ptr<Animation>> m_animations;
    bgfx::ProgramHandle m_program;
    bgfx::ProgramHandle m_erosion;
    bgfx::UniformHandle m_texture;
    bgfx::UniformHandle m_gradient;
public:
    Engine();

    static Engine& Inst();
    Camera& Cam() { return m_camera; }
    void Tick(float time);

    static DrawContext & Ctx();
    void Resize(int w, int h);
    void Draw(DrawContext & nvg);
    void LoadResources(DrawContext & nvg);
    void AddAnimation(const std::shared_ptr<Animation>& anim);
    const std::shared_ptr<SceneGroup> &Root() { return m_root; }
};

