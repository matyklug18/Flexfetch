// vim: ft=c

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdint.h>

Atom ATOM_UTF8_STRING;

Atom ATOM_EWMH_WM_NAME;
Atom ATOM_EWMH_SUPPORTING_WM_CHECK;

Display* xwrap_dpy;

void xwrap_init() {
	xwrap_dpy = XOpenDisplay(NULL);
	ATOM_UTF8_STRING              = XInternAtom(xwrap_dpy,"UTF8_STRING",False);
	ATOM_EWMH_WM_NAME             = XInternAtom(xwrap_dpy,"_NET_WM_NAME",False);
	ATOM_EWMH_SUPPORTING_WM_CHECK = XInternAtom(xwrap_dpy,"_NET_SUPPORTING_WM_CHECK",False);
}

uint8_t* prop_get(Window win, char* name, Atom type, long size) {
	Atom restype;
	int format;
	uint64_t nitems, after;
	uint8_t* data;
	XGetWindowProperty(xwrap_dpy, win, XInternAtom(xwrap_dpy,name,False), 0, size,
		False, type, &restype, &format, &nitems, &after, &data);
	return data;
}

Window prop_get_window(Window win, char* name) {
	return *((Window*)prop_get(win, name, XA_WINDOW, sizeof(Window)));
}

char* prop_get_str(Window win, char* name) {
	return (char*)prop_get(win, name, ATOM_UTF8_STRING, sizeof(char*));
}
