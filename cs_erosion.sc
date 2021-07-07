#include <bgfx_compute.sh>
#include "uniforms.sh"

IMAGE2D_RO(s_texColor, rgba16f, 0);
IMAGE2D_WR(s_target, rgba16f, 1);

NUM_THREADS(16, 16, 1)
void main()
{
	vec4 thispixel = imageLoad(s_texColor, ivec2(gl_GlobalInvocationID.xy) );
	vec4 thispixel_r = imageLoad(s_texColor, ivec2(gl_GlobalInvocationID.xy + ivec2(1, 0)) );
	vec4 thispixel_b = imageLoad(s_texColor, ivec2(gl_GlobalInvocationID.xy + ivec2(0, 1)) );

	vec3 nrm = normalize(vec3(thispixel_r.r - thispixel.r, thispixel_b.r - thispixel.r, .1));
    vec4 grd = vec4((nrm.xy + vec2(1, 1)) * 0.5, 0, 0);	
	const float newWaterAmt = 0.1;
	const float evaporation = 0.1;
	const float speed = 1.0;
	thispixel.g *= evaporation;
	thispixel.g += newWaterAmt;

	vec2 frompixel = gl_GlobalInvocationID.xy  + nrm.xy * speed;
	
	imageStore(s_target, ivec2(gl_GlobalInvocationID.xy), thispixel);
}