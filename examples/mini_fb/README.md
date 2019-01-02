# minifb Example

This example uses [MiniFB](https://github.com/emoon/minifb) to draw the LCD.
Currently only X11 is supported, since that's the only target I can test with. I
also only imported a small part of MiniFB, but it should be simple to add
support for other platforms already provided by MiniFB.

This example implementation is only a **proof of concept**. This is because it
lacks support for:
- Input
- Audio
- Save file
- Everything else that isn't LCD

So don't bother using it for actually playing games, because you can't.

You may be able to use this example as a demonstration of the minimum required
to work with Peanut-GB.
