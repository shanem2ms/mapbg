$input v_texcoord0

/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bgfx_shader.sh>

SAMPLER2D(s_terrain, 0);

void main()
{
		vec4 palette[] =
			{
				vec4(-1000, 0, 10, 160),
				vec4(-0.4, 0, 15, 190),
				vec4(-0.2, 0, 0, 255),
				vec4(-0.1, 0, 128, 192),
				vec4(0, 239, 228, 176),
				vec4(0.1, 128, 64, 0),
				vec4(0.2, 0, 128, 0),
				vec4(0.5, 192, 192, 192),
				vec4(0.7, 220, 240, 240),
				vec4(0.8, 240, 255, 255),
				vec4(100, 255, 0, 0)
			};

		vec2 ts = textureSize(s_terrain, 0);
        vec2 v2 = texture2DLod(s_terrain, v_texcoord0.xy, 0);
		float val = v2.r;
		float val0 = texture2DLod(s_terrain, v_texcoord0.xy - vec2(1.0 / ts.x, 0), 0);
		float val1 = texture2DLod(s_terrain, v_texcoord0.xy - vec2(0, 1.0 / ts.y), 0);
		
		vec3 nrm = normalize(vec3(val - val0, val - val1, 0.05));
		
		float mul = max(0, dot(vec3(0, 0, 1), nrm));
		if (val < 0) mul = 1;
		int pIdx = 0;
		for (; palette[pIdx].r < val && pIdx < 11; ++pIdx);
		pIdx--;
		float m = 1 / 255.0 * mul;

		gl_FragColor = vec4( 		
			palette[pIdx].g * m,
			palette[pIdx].b * m,
			palette[pIdx].a * m,
			1);
} 
