#include <bgfx_compute.sh>
#include "uniforms.sh"

SAMPLER2D(s_texColor, 0);
IMAGE2D_WR(s_target, r16f, 1);

NUM_THREADS(16, 16, 1)
void main()
{
	vec2 div = vec2(1/256.0, 1/256.0);
	vec2 adj = div * 0.5;
	
	float thispixel = texture2DLod(s_texColor, vec2(gl_GlobalInvocationID.xy) * div + adj, 0);
	float thispixel_r = texture2DLod(s_texColor, (vec2(gl_GlobalInvocationID.xy) + vec2(1, 0)) * div + adj, 0);
	float thispixel_b = texture2DLod(s_texColor, (vec2(gl_GlobalInvocationID.xy) + vec2(0, 1)) * div + adj, 0);

	vec3 nrm = normalize(vec3(thispixel_r - thispixel, thispixel_b - thispixel, .1));
    vec4 grd = vec4((nrm.xy + vec2(1, 1)) * 0.5, 0, 0);	
	const float newWaterAmt = 0.1;
	const float evaporation = 0.45;
	const float speed = 3.0;

	imageStore(s_target, ivec2(gl_GlobalInvocationID.xy), thispixel);
}