#include "Engine.h"

Engine::Engine() :
    m_h(0),
    m_w(0),
    m_root(std::make_shared<SceneGroup>())
{
}

void Engine::Resize(int w, int h)
{
    m_h = h;
    m_w = w;
}

void Engine::Tick(float time)
{
    for (auto itAnim = m_animations.begin();
        itAnim != m_animations.end();)
    {
        if ((*itAnim)->ProcessTick(time))
            itAnim++;
        else
            itAnim = m_animations.erase(itAnim);
    }
}

void Engine::Draw(DrawContext & nvg)
{
    m_camera.Update(m_w, m_h);
    DrawContext dc;
    gmtl::identity(dc.m_mat);
    m_root->Draw(dc);
}


void Engine::AddAnimation(const std::shared_ptr<Animation>& anim)
{
    m_animations.push_back(anim);
}

void Engine::LoadResources(DrawContext & nvg)
{
}


Animation::Animation() :
    m_startTime(-1)
{}

bool Animation::ProcessTick(float fullTime)
{
    if (m_startTime < 0)
        m_startTime = fullTime;
    
    return Tick(fullTime - m_startTime);
}


