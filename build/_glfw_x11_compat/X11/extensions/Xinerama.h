#ifndef _XINERAMA_H_COMPAT
#define _XINERAMA_H_COMPAT
#include <X11/Xlib.h>
typedef struct { int screen_number, x_org, y_org, width, height; } XineramaScreenInfo;
#endif
