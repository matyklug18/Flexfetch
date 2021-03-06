# Flexfetch

A fast and generic fetch program

## Screenshots
![Gentoo](./flexfetch-preview.png)

### Features

#### Generic
WM and Compositor are fetched using Xlib as specified in [EWMH](https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html).

Shell is fetched by directly reading the password database (`man 3 getpwnam`).

Distro, Kernel, and System are fetched by reading files in `/etc`, `/proc`, and `/sys`.

Terminal is fetched as the `comm` of the first parent process that has a font open (`/usr/share/fonts`)

Terminal Font is fetched by reading the open fonts of the terminal, finding the last one, and asking `fontconfig` for its name.

GTK Theme is fetched by asking for the `gtk-theme-name` property of gtk settings.

#### Fast
Currently 4 times faster than neofetch, 10 times with the `GTK Theme` module is disabled, and 100 times for the `nolib` version.


### Installation Instructions

1. Copy logo files in `logos/` over to `/usr/share/flexfetch/logos`
2. Compile with `./flexfetch.c` (or `./flexfetch-nox.c` when not running X)
3. Add the resulting binary onto PATH

