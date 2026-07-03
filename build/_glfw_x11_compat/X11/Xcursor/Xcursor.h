#ifndef _XCURSOR_H_COMPAT
#define _XCURSOR_H_COMPAT
#include <X11/Xlib.h>
typedef struct _XcursorImage { unsigned int width, height, xhot, yhot; unsigned char *pixels; unsigned long delay; } XcursorImage;
typedef XcursorImage XcursorCursor;
#endif
