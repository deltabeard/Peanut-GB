# minifb Example

This example uses [MiniFB](https://github.com/emoon/minifb) to draw the LCD.
It now supports both X11 and Linux Framebuffer backends. I also
only imported a small part of MiniFB, but it should be simple to add support
for other platforms already provided by MiniFB.

This example implementation is only a **proof of concept**. This is because it
lacks support for:
- Input
- Audio
- Save file
- Everything else that isn't LCD

So don't bother using it for actually playing games, because you can't.

You may be able to use this example as a demonstration of the minimum required
to work with Peanut-GB.

# build

```bash
# For X11
make x11
# For Linux Framebuffer
# FB might be used on some low-performance embedded devices.
# If you are using it on a desktop, you can switch to
# non-desktop mode to view it.
# e.g., `Ctrl+Alt+F3`, return to desktop use `Ctrl+Alf+F2`
make fb
```