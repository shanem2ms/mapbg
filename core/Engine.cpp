#include "Engine.h"
#include <bx/readerwriter.h>
#include <bx/file.h>

namespace sam
{
    static Engine* sEngine = nullptr;
    Engine::Engine() :
        m_h(0),
        m_w(0),
        m_root(std::make_shared<SceneGroup>())
    {
        sEngine = this;
    }

    void Engine::Resize(int w, int h)
    {
        m_h = h;
        m_w = w;
    }

    void Engine::Tick(float time)
    {
        m_camera.Update(m_w, m_h);

        for (auto itAnim = m_animations.begin();
            itAnim != m_animations.end();)
        {
            if ((*itAnim)->ProcessTick(time))
                itAnim++;
            else
                itAnim = m_animations.erase(itAnim);
        }
    }

    void Engine::Draw(DrawContext& dc)
    {
        dc.m_pgm = m_program;
        dc.m_texture = m_texture;
        dc.m_compute = m_erosion;
        dc.m_gradient = m_gradient;
        gmtl::identity(dc.m_mat);
        m_root->Draw(dc);
    }


    void Engine::AddAnimation(const std::shared_ptr<Animation>& anim)
    {
        m_animations.push_back(anim);
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

    static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
    {
        if (bx::open(_reader, _filePath))
        {
            uint32_t size = (uint32_t)bx::getSize(_reader);
            const bgfx::Memory* mem = bgfx::alloc(size + 1);
            bx::read(_reader, mem->data, size);
            bx::close(_reader);
            mem->data[mem->size - 1] = '\0';
            return mem;
        }

        return NULL;
    }

    void Engine::LoadResources(DrawContext& nvg)
    {
        bx::FileReader fileReader;
        bgfx::ShaderHandle vtxShader = bgfx::createShader(loadMem(&fileReader, "vs_cubes.bin"));
        bgfx::ShaderHandle fragShader = bgfx::createShader(loadMem(&fileReader, "fs_cubes.bin"));
        m_program = bgfx::createProgram(vtxShader, fragShader, true);
        m_texture = bgfx::createUniform("s_terrain", bgfx::UniformType::Sampler);
        m_gradient = bgfx::createUniform("s_gradient", bgfx::UniformType::Sampler);
        bgfx::ShaderHandle erosion = bgfx::createShader(loadMem(&fileReader, "cs_erosion.bin"));
        m_erosion = bgfx::createProgram(erosion, true);
    }

    Engine& Engine::Inst()
    {
        return *sEngine;
    }
}