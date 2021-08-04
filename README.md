# Flexfetch

A fast fetch program

### Features

#### Generic
WM and Compositor are fetched using Xlib as specified in EWMH.

Shell is fetched by directly reading the password database (man 3 getpwnam).

Distro, Kernel, and System are fetched by reading files in `/etc`, `/proc`, and `/sys`.

Terminal is fetched as the `comm` of the first parent process that has a font open (`/usr/share/fonts`)

Terminal Font is fetched by reading the open fonts of the terminal, and asking fontconfig for their names.

GTK Theme is fetched by asking for the `gtk-theme-name` property of gtk settings.

#### Fast
Currently 4 times faster than neofetch, or 10 times with the `GTK Theme` module is disabled (a fix is in order).

### Installation Instructions

1. Copy logo files in `logos/` over to /usr/share/flexfetch/logos
2. Compile with `./flexfetch.c` (or `./flexfetch-nox.c` when not running X)
3. Add the resulting binary onto PATH
