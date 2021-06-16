#pragma once


namespace hsl
{
    typedef struct {
        float r;       // a fraction between 0 and 1
        float g;       // a fraction between 0 and 1
        float b;       // a fraction between 0 and 1
    } rgb;

    typedef struct {
        float h;       // angle in degrees
        float s;       // a fraction between 0 and 1
        float l;       // a fraction between 0 and 1
    } hsl;

    hsl to_hsl(rgb in);
    rgb to_rgb(hsl in);
    hsl lerp(const hsl& l, const hsl& r, float t);
    rgb lerp(const rgb& l, const rgb& r, float t);
}