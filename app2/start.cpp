/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#define ENTRY_CONFIG_IMPLEMENT_MAIN 1

#include <bx/uint32_t.h>
#include "entry.h"
#include "bgfx_utils.h"
#include "Application.h"

namespace
{

class ExampleHelloWorld : public entry::AppI
{
public:
	ExampleHelloWorld(const char* _name, const char* _description, const char* _url)
		: entry::AppI(_name, _description, _url)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height,
              const char *docPath) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_TEXT;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
			);
        
        app.SetDocPath(docPath);
        app.Resize(_width, _height);
	}

	virtual int shutdown() override
	{
		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
        int prevheight = m_height;
        int prevwidth = m_width;
        static uint8_t prevMouseLeftBtn = 0;
        static float prevX = 0, prevY = 0;
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
            if (prevheight != m_height ||
                prevwidth != m_width)
                app.Resize(m_width, m_height);
            
            if (m_mouseState.m_buttons[entry::MouseButton::Left] != prevMouseLeftBtn)
            {
                prevMouseLeftBtn = m_mouseState.m_buttons[entry::MouseButton::Left];
                
                if (prevMouseLeftBtn > 0)
                    app.TouchDown(m_mouseState.m_mx, m_mouseState.m_my, 0);
                else
                    app.TouchUp(0);
            }
            else if (prevMouseLeftBtn > 0 && (prevX != m_mouseState.m_mx
                                              || prevY != m_mouseState.m_my))
            {
                app.TouchDrag(m_mouseState.m_mx, m_mouseState.m_my, 0);
            }
            prevX = m_mouseState.m_mx;
            prevY = m_mouseState.m_my;
			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);
            
            app.Tick(0);
            app.Draw();
			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
    sam::Application app;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	  ExampleHelloWorld
	, "00-helloworld"
	, "Initialization and debug text."
	, "https://bkaradzic.github.io/bgfx/examples.html#helloworld"
	);
