$input a_position
$input a_texcoord0
$output v_texcoord0

/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */ 


#include <bgfx_shader.sh>

SAMPLER2D(s_terrain, 0);

void main()
{
	float val = texture2DLod(s_terrain, a_texcoord0.xy, 0);
	if (val > 900)
		val -= 1000;
	v_texcoord0 = a_texcoord0;
	val = max(val, 0);
	gl_Position = mul(u_modelViewProj, vec4(a_position.x, a_position.y, a_position.z - val * 0.5, 1.0) );
}
 