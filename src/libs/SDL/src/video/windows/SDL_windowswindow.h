/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#ifndef _SDL_windowswindow_h
#define _SDL_windowswindow_h

#if SDL_VIDEO_OPENGL_EGL   
#include "../SDL_egl_c.h"
#endif

typedef struct
{
    SDL_Window *window;
    HWND hwnd;
    HDC hdc;
    HDC mdc;
    HBITMAP hbm;
    WNDPROC wndproc;
    SDL_bool created;
    WPARAM mouse_button_flags;
    SDL_bool expected_resize;
    SDL_bool in_border_change;
    SDL_bool in_title_click;
    SDL_bool in_modal_loop;
    struct SDL_VideoData *videodata;
#if SDL_VIDEO_OPENGL_EGL  
    EGLSurface egl_surface;
#endif
} SDL_WindowData;

extern int WIN_CreateWindow(_THIS, SDL_Window * window);
extern int WIN_CreateWindowFrom(_THIS, SDL_Window * window, const void *data);
extern void WIN_SetWindowTitle(_THIS, SDL_Window * window);
extern void WIN_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon);
extern void WIN_SetWindowPosition(_THIS, SDL_Window * window);
extern void WIN_SetWindowSize(_THIS, SDL_Window * window);
extern void WIN_ShowWindow(_THIS, SDL_Window * window);
extern void WIN_HideWindow(_THIS, SDL_Window * window);
extern void WIN_RaiseWindow(_THIS, SDL_Window * window);
extern void WIN_MaximizeWindow(_THIS, SDL_Window * window);
extern void WIN_MinimizeWindow(_THIS, SDL_Window * window);
extern void WIN_RestoreWindow(_THIS, SDL_Window * window);
extern void WIN_SetWindowBordered(_THIS, SDL_Window * window, SDL_bool bordered);
extern void WIN_SetWindowFullscreen(_THIS, SDL_Window * window, SDL_VideoDisplay * display, SDL_bool fullscreen);
extern int WIN_SetWindowGammaRamp(_THIS, SDL_Window * window, const Uint16 * ramp);
extern int WIN_GetWindowGammaRamp(_THIS, SDL_Window * window, Uint16 * ramp);
extern void WIN_SetWindowGrab(_THIS, SDL_Window * window, SDL_bool grabbed);
extern void WIN_DestroyWindow(_THIS, SDL_Window * window);
extern SDL_bool WIN_GetWindowWMInfo(_THIS, SDL_Window * window,
                                    struct SDL_SysWMinfo *info);
extern void WIN_OnWindowEnter(_THIS, SDL_Window * window);
extern void WIN_UpdateClipCursor(SDL_Window *window);

#endif /* _SDL_windowswindow_h */

/* vi: set ts=4 sw=4 expandtab: */
