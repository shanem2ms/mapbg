#include "HSLColor.h"
#include <math.h>

namespace hsl
{
    hsl to_hsl(rgb in)
    {
        hsl         out;
        float      min, max, delta;

        min = in.r < in.g ? in.r : in.g;
        min = min < in.b ? min : in.b;

        max = in.r > in.g ? in.r : in.g;
        max = max > in.b ? max : in.b;

        out.l = max;                                // v
        delta = max - min;
        if (delta < 0.00001)
        {
            out.s = 0;
            out.h = 0; // undefined, maybe nan?
            return out;
        }
        if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
            out.s = (delta / max);                  // s
        }
        else {
            // if max is 0, then r = g = b = 0              
            // s = 0, h is undefined
            out.s = 0.0;
            out.h = NAN;                            // its now undefined
            return out;
        }
        if (in.r >= max)                           // > is bogus, just keeps compilor happy
            out.h = (in.g - in.b) / delta;        // between yellow & magenta
        else
            if (in.g >= max)
                out.h = 2.0f + (in.b - in.r) / delta;  // between cyan & yellow
            else
                out.h = 4.0f + (in.r - in.g) / delta;  // between magenta & cyan

        out.h *= (1.0f / 6.0f);

        if (out.h < 0.0f)
            out.h += 1;

        return out;
    }

    rgb to_rgb(hsl in)
    {
        float      hh, p, q, t, ff;
        long        i;
        rgb         out;

        if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
            out.r = in.l;
            out.g = in.l;
            out.b = in.l;
            return out;
        }
        hh = in.h;
        if (hh >= 1.0) hh = 0.0;
        hh *= 6.0;
        i = (long)hh;
        ff = hh - i;
        p = in.l * (1.0f - in.s);
        q = in.l * (1.0f - (in.s * ff));
        t = in.l * (1.0f - (in.s * (1.0f - ff)));

        switch (i) {
        case 0:
            out.r = in.l;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = in.l;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = in.l;
            out.b = t;
            break;

        case 3:
            out.r = p;
            out.g = q;
            out.b = in.l;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = in.l;
            break;
        case 5:
        default:
            out.r = in.l;
            out.g = p;
            out.b = q;
            break;
        }
        return out;
    }

    hsl lerp(const hsl& l, const hsl& r, float t)
    {
       return hsl{ l.h * (1 - t) + r.h * t,
            l.s * (1 - t) + r.s * t,
            l.l * (1 - t) + r.l * t };
    }

    rgb lerp(const rgb& l, const rgb& r, float t)
    {
        return rgb{ l.r * (1 - t) + r.r * t,
             l.g * (1 - t) + r.g * t,
             l.b * (1 - t) + r.b * t };
    }
}
