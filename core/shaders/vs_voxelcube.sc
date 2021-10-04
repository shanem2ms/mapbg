$input a_position, a_normal, a_texcoord0, i_data0
$output v_texcoord0, v_normal


/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */ 


#include <bgfx_shader.sh>

void main()
{ 
	vec2 tx = a_texcoord0;
	v_texcoord0 = tx; 
	v_normal = a_normal;  
	int ix = i_data0.x - 127;
	int iy = i_data0.y - 127;
	int iz = i_data0.z - 127;
	gl_Position = mul(u_modelViewProj, vec4(a_position.x + ix, a_position.y + iy, a_position.z + iz, 1.0) );
} 
  