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
	vec2 tx = a_texcoord0 * 2.0/3.0 + vec2(1.0/6.0, 1.0/6.0);
	float val = texture2DLod(s_terrain, tx.xy, 0).a;
	if (val > 900)
		val -= 1000;
	v_texcoord0 = tx;
	val = max(val, 0);
	gl_Position = mul(u_modelViewProj, vec4(a_position.x, a_position.y, a_position.z - val, 1.0) );
}
 