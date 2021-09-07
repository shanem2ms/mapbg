#pragma once
#include <string>
#include <vector>
#include <map>

#include <coroutine>

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/when_all.hpp>

#include <bgfx/bgfx.h>

extern cppcoro::static_thread_pool g_threadPool;

namespace cppcoro
{
    template <typename Func>
    task<> dispatch(Func func) {
        co_await g_threadPool.schedule();
        co_await func();
    }
}

template <class T> class bgfxh
{
    T t;
public:
    bgfxh() :
        t(BGFX_INVALID_HANDLE)
    {

    }

    operator T() const
    {
        return t;
    }

    T &operator = (const T& rhs)
    {
        t = rhs;
        return t;
    }

    void free()
    {
        if (bgfx::isValid(t))
        {
            bgfx::destroy(t);
            t = BGFX_INVALID_HANDLE;
        }
    }
    ~bgfxh()
    {
        free();
    }

private:
    bgfxh(const bgfxh& rhs);
};

namespace co = cppcoro;


